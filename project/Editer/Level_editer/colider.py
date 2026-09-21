import bpy
import gpu
import gpu_extras.batch
import copy
import math
import mathutils

# ==========================================
# コライダー ＆ レイキャスト自動表示・地面埋まり防止モジュール (colider.py)
# ==========================================

# ------------------------------------------
# 1. 地面レイキャスト判定ユーティリティ
# ------------------------------------------

def get_object_bottom_z(obj):
    """
    オブジェクトの底面（最下面）のワールドZ座標を取得する。
    コライダーがあればコライダーの底面、なければバウンディングボックスの底面。
    """
    if not obj:
        return 0.0

    mat = obj.matrix_world

    # コライダープロパティがある場合
    if "collider" in obj and "collider_center" in obj and "collider_size" in obj:
        c_center = mathutils.Vector((
            obj["collider_center"][0],
            obj["collider_center"][1],
            obj["collider_center"][2]
        ))
        c_size = mathutils.Vector((
            obj["collider_size"][0],
            obj["collider_size"][1],
            obj["collider_size"][2]
        ))
        bottom_local = c_center - mathutils.Vector((0.0, 0.0, c_size.z * 0.5))
        bottom_world = mat @ bottom_local
        return bottom_world.z

    # メッシュのバウンディングボックス
    if obj.type == 'MESH' and obj.bound_box:
        min_z = min((mat @ mathutils.Vector(corner)).z for corner in obj.bound_box)
        return min_z

    # その他（原点位置 - 0.5m）
    return mat.translation.z - 0.5


def ray_cast_to_ground(scene, depsgraph, origin_xy, start_z=100.0, max_dist=250.0, ignore_objs=None):
    """
    指定の (X, Y) 座標において、上空から真下へレイキャストを飛ばし、
    地面（TerrainGround、BLOCK、TERRAIN、その他メッシュ）の表面座標と法線を取得する。
    returns: (hit: bool, hit_location: Vector, hit_normal: Vector, hit_object: Object)
    """
    if ignore_objs is None:
        ignore_objs = set()

    origin = mathutils.Vector((origin_xy[0], origin_xy[1], start_z))
    direction = mathutils.Vector((0.0, 0.0, -1.0))
    remaining_dist = max_dist

    while remaining_dist > 0.01:
        hit, loc, norm, idx, hit_obj, mat = scene.ray_cast(
            depsgraph,
            origin,
            direction,
            distance=remaining_dist
        )
        if not hit:
            break

        # 無視対象（自分自身など）やカーブ、カメラ、ライトは除外
        if hit_obj not in ignore_objs and hit_obj.type == 'MESH':
            # 地面として有効なメッシュにヒット
            return True, loc, norm, hit_obj

        # 無視対象に当たった場合は少し進めて再レイキャスト
        step = (loc - origin).length + 0.05
        remaining_dist -= step
        origin = loc + direction * 0.05

    return False, None, None, None


def snap_object_to_ground_surface(obj, scene, depsgraph):
    """
    オブジェクトの底面をレイキャストで検知した地面の表面にピッタリ合わせ、
    地面への埋まりを自動解消する。
    returns: (success: bool, offset_applied: float)
    """
    if not obj or obj.type in ['CURVE', 'CAMERA', 'LIGHT']:
        return False, 0.0

    w_pos = obj.matrix_world.translation.copy()
    bottom_z = get_object_bottom_z(obj)

    # 自分自身と子オブジェクトを無視リストへ
    ignore = {obj}
    for child in obj.children:
        ignore.add(child)

    # オブジェクトの中心やや上から真下にレイキャスト
    start_z = max(w_pos.z + 10.0, 50.0)
    hit, ground_loc, ground_norm, hit_obj = ray_cast_to_ground(
        scene, depsgraph, (w_pos.x, w_pos.y), start_z=start_z, ignore_objs=ignore
    )

    if not hit or ground_loc is None:
        return False, 0.0

    target_ground_z = ground_loc.z
    # 底面と地面表面の差分
    diff_z = target_ground_z - bottom_z

    # 埋まっている（diff_z > 0）、あるいはわずかな浮きを接地させる
    new_w_pos = mathutils.Vector((w_pos.x, w_pos.y, w_pos.z + diff_z))

    if obj.parent:
        obj.location = obj.parent.matrix_world.inverted() @ new_w_pos
    else:
        obj.location = new_w_pos

    # rail_pos を持っている場合は高さ y も同期更新
    if "rail_pos" in obj:
        rp = list(obj["rail_pos"])
        if len(rp) > 1:
            try:
                from .rail_snap import find_target_rail_object, get_rail_world_position
                rail = find_target_rail_object(obj.get("rail_target"))
                if rail:
                    rail_pt = get_rail_world_position(rail, rp[0])
                    if rail_pt:
                        rp[1] = round(new_w_pos.z - rail_pt.z, 3)
                        obj["rail_pos"] = rp
            except Exception:
                pass

    return True, diff_z


# ------------------------------------------
# 2. オペレーター群
# ------------------------------------------

class MYADDON_OT_add_collider(bpy.types.Operator):
    bl_idname = "myaddon.myaddon_ot_add_collider"
    bl_label = "Collider 追加"
    bl_description = "['collider'] カスタムプロパティを追加し、コライダー形状を設定します"
    bl_options = {"REGISTER", "UNDO"}

    collider_type: bpy.props.EnumProperty(
        name="形状",
        items=[
            ("BOX", "ボックス (Box)", "直方体コライダー"),
            ("SPHERE", "スフィア (Sphere)", "球体コライダー"),
        ],
        default="BOX",
    )

    def execute(self, context):
        obj = context.object
        if not obj:
            return {"CANCELLED"}
        obj["collider"] = self.collider_type
        obj["collider_center"] = mathutils.Vector((0.0, 0.0, 0.0))
        obj["collider_size"] = mathutils.Vector((2.0, 2.0, 2.0))
        self.report({'INFO'}, f"コライダー ({self.collider_type}) を追加しました。")
        return {"FINISHED"}


class MYADDON_OT_snap_to_ground(bpy.types.Operator):
    bl_idname = "myaddon.snap_to_ground"
    bl_label = "地面に接地（埋まり解消）"
    bl_description = "レイキャストにより直下の地面表面を検知し、オブジェクトが地面に埋まらないようピッタリ接地させます"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        obj = context.object
        if not obj:
            self.report({'WARNING'}, "オブジェクトが選択されていません。")
            return {'CANCELLED'}

        depsgraph = context.evaluated_depsgraph_get()
        ok, diff = snap_object_to_ground_surface(obj, context.scene, depsgraph)
        if ok:
            self.report({'INFO'}, f"'{obj.name}' を地面表面に接地させました (補正量: {diff:+.3f}m)")
            return {'FINISHED'}
        else:
            self.report({'WARNING'}, "直下に地面メッシュ（TerrainGround/BLOCK等）が見つかりませんでした。")
            return {'CANCELLED'}


class MYADDON_OT_snap_all_to_ground(bpy.types.Operator):
    bl_idname = "myaddon.snap_all_to_ground"
    bl_label = "全オブジェクトの埋まりを一括解消"
    bl_description = "シーン内のすべてのエネミー、プレイヤー、ゴール等の埋まりをレイキャストで検知し、一括で地面に接地させます"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        depsgraph = context.evaluated_depsgraph_get()
        scene = context.scene
        count = 0

        for obj in scene.objects:
            if obj.type in ['CURVE', 'CAMERA', 'LIGHT']:
                continue
            if obj.get("object_type") in ["TERRAIN"]:
                continue
            # 地面オブジェクト自体はスキップ
            if "terrain" in obj.name.lower() or obj.name.startswith("Road_"):
                continue

            is_target = ("collider" in obj) or ("rail_pos" in obj) or (obj.get("object_type") in ["ENEMY", "PLAYER_SPAWN", "GOAL", "PROP"])
            if not is_target:
                continue

            ok, diff = snap_object_to_ground_surface(obj, scene, depsgraph)
            if ok and abs(diff) > 0.001:
                count += 1

        self.report({'INFO'}, f"{count} 個のオブジェクトの地面埋まりを解消しました！")
        return {'FINISHED'}


# ------------------------------------------
# 3. リアルタイム地面埋まり防止ハンドラー
# ------------------------------------------

_is_ground_snapping = False

@bpy.app.handlers.persistent
def ground_auto_snap_handler(scene, depsgraph):
    """
    オブジェクト移動時に地面に埋まっている場合、自動で表面に押し上げるリアルタイムハンドラー
    """
    global _is_ground_snapping
    if _is_ground_snapping:
        return
    if not getattr(scene, "auto_ground_snap", True):
        return

    active_obj = None
    try:
        active_obj = bpy.context.view_layer.objects.active
    except Exception:
        pass

    if not active_obj or active_obj.mode != 'OBJECT':
        return

    # 対象判定
    if active_obj.type in ['CURVE', 'CAMERA', 'LIGHT']:
        return
    if active_obj.get("object_type") == "TERRAIN" or "terrain" in active_obj.name.lower():
        return

    is_target = ("collider" in active_obj) or ("rail_pos" in active_obj) or (active_obj.get("object_type") in ["ENEMY", "PLAYER_SPAWN", "GOAL", "PROP", "BLOCK"])
    if not is_target:
        return

    # トランスフォームが更新されたかチェック
    updated = False
    for update in depsgraph.updates:
        if update.id.original == active_obj and update.is_updated_transform:
            updated = True
            break
    if not updated:
        return

    # 底面と地面をチェック
    bottom_z = get_object_bottom_z(active_obj)
    w_pos = active_obj.matrix_world.translation

    ignore = {active_obj}
    for c in active_obj.children:
        ignore.add(c)

    hit, ground_loc, _, _ = ray_cast_to_ground(
        scene, depsgraph, (w_pos.x, w_pos.y), start_z=max(w_pos.z + 10.0, 50.0), ignore_objs=ignore
    )

    if hit and ground_loc is not None:
        target_ground_z = ground_loc.z
        # もし地面に埋まっている（底面が地面表面より下にある）場合
        if bottom_z < (target_ground_z - 0.005):
            _is_ground_snapping = True
            try:
                diff_z = target_ground_z - bottom_z
                new_w_pos = mathutils.Vector((w_pos.x, w_pos.y, w_pos.z + diff_z))
                if active_obj.parent:
                    active_obj.location = active_obj.parent.matrix_world.inverted() @ new_w_pos
                else:
                    active_obj.location = new_w_pos
            finally:
                _is_ground_snapping = False


# ------------------------------------------
# 4. コライダー ＆ レイキャスト自動 3Dビュー描画
# ------------------------------------------

class DrawCollider:
    handle = None

    @staticmethod
    def _create_box_lines(center, size, mat):
        """直方体ワイヤーフレームの頂点とインデックスを生成"""
        offsets = [
            [-0.5, -0.5, -0.5], [+0.5, -0.5, -0.5],
            [-0.5, +0.5, -0.5], [+0.5, +0.5, -0.5],
            [-0.5, -0.5, +0.5], [+0.5, -0.5, +0.5],
            [-0.5, +0.5, +0.5], [+0.5, +0.5, +0.5],
        ]
        pts = []
        for ofs in offsets:
            p = mathutils.Vector((
                center.x + ofs[0] * size.x,
                center.y + ofs[1] * size.y,
                center.z + ofs[2] * size.z,
            ))
            pts.append(mat @ p)

        edges = [
            (0, 1), (2, 3), (0, 2), (1, 3),
            (4, 5), (6, 7), (4, 6), (5, 7),
            (0, 4), (1, 5), (2, 6), (3, 7),
        ]
        return pts, edges

    @staticmethod
    def _create_sphere_lines(center, radius, mat, segments=16):
        """球体ワイヤーフレーム（3大円）の頂点とインデックスを生成"""
        pts = []
        edges = []
        # XY円
        start = len(pts)
        for i in range(segments):
            ang = 2.0 * math.pi * (i / segments)
            p = mathutils.Vector((center.x + radius * math.cos(ang), center.y + radius * math.sin(ang), center.z))
            pts.append(mat @ p)
            edges.append((start + i, start + ((i + 1) % segments)))
        # XZ円
        start = len(pts)
        for i in range(segments):
            ang = 2.0 * math.pi * (i / segments)
            p = mathutils.Vector((center.x + radius * math.cos(ang), center.y, center.z + radius * math.sin(ang)))
            pts.append(mat @ p)
            edges.append((start + i, start + ((i + 1) % segments)))
        # YZ円
        start = len(pts)
        for i in range(segments):
            ang = 2.0 * math.pi * (i / segments)
            p = mathutils.Vector((center.x, center.y + radius * math.cos(ang), center.z + radius * math.sin(ang)))
            pts.append(mat @ p)
            edges.append((start + i, start + ((i + 1) % segments)))
        return pts, edges

    @staticmethod
    def _create_ground_marker(center, radius=0.6, segments=12):
        """地面交点用の円形マーカー"""
        pts = []
        edges = []
        for i in range(segments):
            ang = 2.0 * math.pi * (i / segments)
            pts.append(mathutils.Vector((center.x + radius * math.cos(ang), center.y + radius * math.sin(ang), center.z + 0.02)))
            edges.append((i, (i + 1) % segments))
        # 十字クロス
        idx = len(pts)
        pts.append(mathutils.Vector((center.x - radius, center.y, center.z + 0.02)))
        pts.append(mathutils.Vector((center.x + radius, center.y, center.z + 0.02)))
        edges.append((idx, idx + 1))
        pts.append(mathutils.Vector((center.x, center.y - radius, center.z + 0.02)))
        pts.append(mathutils.Vector((center.x, center.y + radius, center.z + 0.02)))
        edges.append((idx + 2, idx + 3))
        return pts, edges

    @staticmethod
    def draw_colider():
        scene = bpy.context.scene
        draw_col = getattr(scene, "draw_collider_always", True)
        draw_ray = getattr(scene, "draw_raycast_always", True)

        if not draw_col and not draw_ray:
            return

        depsgraph = None
        try:
            depsgraph = bpy.context.evaluated_depsgraph_get()
        except Exception:
            pass

        normal_col_verts = []
        normal_col_indices = []

        buried_col_verts = []
        buried_col_indices = []

        ray_verts = []
        ray_indices = []

        hit_marker_verts = []
        hit_marker_indices = []

        for obj in scene.objects:
            if obj.type in ['CURVE', 'CAMERA', 'LIGHT']:
                continue
            if obj.get("object_type") == "TERRAIN" or "terrain" in obj.name.lower() or obj.name.startswith("Road_"):
                continue

            has_col = "collider" in obj
            is_target = has_col or ("rail_pos" in obj) or (obj.get("object_type") in ["ENEMY", "PLAYER_SPAWN", "GOAL", "PROP"])
            if not is_target:
                continue

            w_pos = obj.matrix_world.translation
            bottom_z = get_object_bottom_z(obj)

            # 地面レイキャスト判定
            is_buried = False
            ground_loc = None

            if depsgraph and draw_ray:
                ignore = {obj}
                for c in obj.children:
                    ignore.add(c)
                hit, g_loc, _, _ = ray_cast_to_ground(
                    scene, depsgraph, (w_pos.x, w_pos.y), start_z=max(w_pos.z + 10.0, 50.0), ignore_objs=ignore
                )
                if hit and g_loc is not None:
                    ground_loc = g_loc
                    # 地面表面より底面が下にあるか
                    if bottom_z < (ground_loc.z - 0.01):
                        is_buried = True

            # ── コライダー描画 ──
            if draw_col and has_col:
                col_type = obj.get("collider", "BOX")
                c_center = mathutils.Vector(obj.get("collider_center", (0.0, 0.0, 0.0)))
                c_size = mathutils.Vector(obj.get("collider_size", (2.0, 2.0, 2.0)))

                if col_type == "SPHERE":
                    radius = max(c_size.x, c_size.y, c_size.z) * 0.5
                    c_pts, c_edges = DrawCollider._create_sphere_lines(c_center, radius, obj.matrix_world)
                else:
                    c_pts, c_edges = DrawCollider._create_box_lines(c_center, c_size, obj.matrix_world)

                target_verts = buried_col_verts if is_buried else normal_col_verts
                target_indices = buried_col_indices if is_buried else normal_col_indices

                v_base = len(target_verts)
                for p in c_pts:
                    target_verts.append((p.x, p.y, p.z))
                for e in c_edges:
                    target_indices.append((v_base + e[0], v_base + e[1]))

            # ── レイキャスト描画（足元から地面へのレイ＆衝突点） ──
            if draw_ray and ground_loc is not None:
                # レイ線: オブジェクト底面 〜 地面表面
                r_base = len(ray_verts)
                ray_verts.append((w_pos.x, w_pos.y, bottom_z))
                ray_verts.append((w_pos.x, w_pos.y, ground_loc.z))
                ray_indices.append((r_base, r_base + 1))

                # 地面衝突点マーカー
                m_pts, m_edges = DrawCollider._create_ground_marker(ground_loc, radius=0.6)
                m_base = len(hit_marker_verts)
                for p in m_pts:
                    hit_marker_verts.append((p.x, p.y, p.z))
                for e in m_edges:
                    hit_marker_indices.append((m_base + e[0], m_base + e[1]))

        shader = gpu.shader.from_builtin("UNIFORM_COLOR")

        # 1. 通常コライダー（シアン色: 0.2, 0.9, 0.9）
        if normal_col_verts:
            batch = gpu_extras.batch.batch_for_shader(
                shader, "LINES", {"pos": normal_col_verts}, indices=normal_col_indices
            )
            shader.bind()
            shader.uniform_float("color", (0.2, 0.9, 0.9, 0.85))
            gpu.state.line_width_set(1.8)
            batch.draw(shader)

        # 2. 埋まり警告コライダー（警告レッド: 1.0, 0.2, 0.2）
        if buried_col_verts:
            batch = gpu_extras.batch.batch_for_shader(
                shader, "LINES", {"pos": buried_col_verts}, indices=buried_col_indices
            )
            shader.bind()
            shader.uniform_float("color", (1.0, 0.2, 0.2, 0.95))
            gpu.state.line_width_set(2.4)
            batch.draw(shader)

        # 3. レイキャストライン（イエローグリーン: 0.3, 1.0, 0.3）
        if ray_verts:
            batch = gpu_extras.batch.batch_for_shader(
                shader, "LINES", {"pos": ray_verts}, indices=ray_indices
            )
            shader.bind()
            shader.uniform_float("color", (0.3, 1.0, 0.4, 0.9))
            gpu.state.line_width_set(1.5)
            batch.draw(shader)

        # 4. 地面着地点マーカー（イエロー: 1.0, 0.9, 0.1）
        if hit_marker_verts:
            batch = gpu_extras.batch.batch_for_shader(
                shader, "LINES", {"pos": hit_marker_verts}, indices=hit_marker_indices
            )
            shader.bind()
            shader.uniform_float("color", (1.0, 0.9, 0.1, 0.95))
            gpu.state.line_width_set(2.0)
            batch.draw(shader)


# ------------------------------------------
# 5. UI パネル
# ------------------------------------------

class OBJECT_PT_collider(bpy.types.Panel):
    bl_idname = "OBJECT_PT_collider"
    bl_label = "Collider & Ground Snapping"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "object"

    @classmethod
    def poll(cls, context):
        obj = context.object
        return obj is not None and obj.type != 'CURVE'

    def draw(self, context):
        layout = self.layout
        obj = context.object
        scene = context.scene

        # ── 1. 自動表示 ＆ リアルタイム埋まり防止トグル ──
        box_view = layout.box()
        box_view.label(text="🎯 自動表示 ＆ 地面埋まり防止:", icon='RESTRICT_VIEW_OFF')
        row_tgl = box_view.row(align=True)
        row_tgl.prop(scene, "draw_collider_always", text="コライダー表示", icon='SHADING_WIRE')
        row_tgl.prop(scene, "draw_raycast_always", text="レイキャスト表示", icon='LIGHT_SUN')

        row_snap = box_view.row()
        row_snap.prop(scene, "auto_ground_snap", text="🧲 自動で地面に接地（埋まり防止）", icon='SNAP_ON')

        # ── 2. 接地アクション ──
        box_act = layout.box()
        box_act.label(text="接地・埋まり解消アクション:")
        row_btn = box_act.row(align=True)
        row_btn.operator(MYADDON_OT_snap_to_ground.bl_idname, text="選択対象を地面に接地", icon='EMPTY_SINGLE_ARROW')
        row_btn.operator(MYADDON_OT_snap_all_to_ground.bl_idname, text="全埋まりを一括解消", icon='FILE_REFRESH')

        layout.separator()

        # ── 3. コライダー設定 ──
        if "collider" in obj:
            box_col = layout.box()
            box_col.label(text="コライダー形状・パラメータ:", icon='MESH_ICOSPHERE')
            box_col.prop(obj, '["collider"]', text="形状 (Type)")
            box_col.prop(obj, '["collider_center"]', text="中心オフセット")
            box_col.prop(obj, '["collider_size"]', text="サイズ (XYZ)")
        else:
            layout.operator(MYADDON_OT_add_collider.bl_idname, text="Collider 追加", icon='ADD')


# ------------------------------------------
# 6. プロパティ登録・解除
# ------------------------------------------

classes = (
    MYADDON_OT_add_collider,
    MYADDON_OT_snap_to_ground,
    MYADDON_OT_snap_all_to_ground,
    OBJECT_PT_collider,
)

def register():
    for cls in classes:
        try:
            bpy.utils.register_class(cls)
        except ValueError:
            pass

    # シーンプロパティ初期化
    bpy.types.Scene.draw_collider_always = bpy.props.BoolProperty(
        name="コライダー自動表示",
        description="3Dビューポートにコライダーのワイヤーフレームを自動常時表示します（埋まっている場合は赤色で警告）",
        default=True,
    )
    bpy.types.Scene.draw_raycast_always = bpy.props.BoolProperty(
        name="レイキャスト自動表示",
        description="足元から地面表面へのレイキャストと接地マーカーを自動常時表示します",
        default=True,
    )
    bpy.types.Scene.auto_ground_snap = bpy.props.BoolProperty(
        name="自動地面接地（埋まり防止）",
        description="オブジェクト移動時に地面メッシュを検知し、地面に埋まらないよう自動的に表面にピッタリ乗せます",
        default=True,
    )

    if ground_auto_snap_handler not in bpy.app.handlers.depsgraph_update_post:
        bpy.app.handlers.depsgraph_update_post.append(ground_auto_snap_handler)

    if DrawCollider.handle is None:
        DrawCollider.handle = bpy.types.SpaceView3D.draw_handler_add(
            DrawCollider.draw_colider, (), "WINDOW", "POST_VIEW"
        )


def unregister():
    if DrawCollider.handle:
        bpy.types.SpaceView3D.draw_handler_remove(DrawCollider.handle, "WINDOW")
        DrawCollider.handle = None

    if ground_auto_snap_handler in bpy.app.handlers.depsgraph_update_post:
        bpy.app.handlers.depsgraph_update_post.remove(ground_auto_snap_handler)

    for prop in ["draw_collider_always", "draw_raycast_always", "auto_ground_snap"]:
        if hasattr(bpy.types.Scene, prop):
            delattr(bpy.types.Scene, prop)

    for cls in reversed(classes):
        try:
            bpy.utils.unregister_class(cls)
        except Exception:
            pass


if __name__ == "__main__":
    register()