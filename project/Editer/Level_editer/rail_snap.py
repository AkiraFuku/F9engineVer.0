import bpy
import math
import mathutils
import gpu
import gpu_extras.batch

# ==========================================
# 1. レールベジェ曲線計算 & スナップユーティリティ
# ==========================================

def evaluate_bezier(p0, p1, p2, p3, t):
    """3次ベジェ曲線の補間点を計算 (mathutils.Vector)"""
    u = 1.0 - t
    uu = u * u
    uuu = uu * u
    tt = t * t
    ttt = tt * t
    return uuu * p0 + 3.0 * uu * t * p1 + 3.0 * u * tt * p2 + ttt * p3


def get_rail_world_position(curve_obj, global_t):
    """
    CURVE オブジェクトのベジェスプラインから、進行度 global_t におけるワールド座標を計算
    """
    if not curve_obj or curve_obj.type != 'CURVE':
        return None
    curve_data = curve_obj.data
    if not curve_data.splines:
        return None
    spline = curve_data.splines[0]
    points = spline.bezier_points
    num_pts = len(points)
    if num_pts == 0:
        return None
    if num_pts == 1:
        return curve_obj.matrix_world @ points[0].co

    is_loop = spline.use_cyclic_u
    max_t = float(num_pts) if is_loop else float(num_pts - 1)

    if is_loop:
        global_t = global_t % max_t
        if global_t < 0.0:
            global_t += max_t
    else:
        if global_t <= 0.0:
            return curve_obj.matrix_world @ points[0].co
        if global_t >= max_t:
            return curve_obj.matrix_world @ points[-1].co

    idx1 = int(global_t)
    if is_loop:
        idx2 = (idx1 + 1) % num_pts
        t = global_t - float(idx1)
    else:
        if idx1 >= num_pts - 1:
            idx1 = num_pts - 2
            idx2 = num_pts - 1
            t = 1.0
        else:
            idx2 = idx1 + 1
            t = global_t - float(idx1)

    p1 = points[idx1]
    p2 = points[idx2]

    # ワールド座標へ変換
    mat = curve_obj.matrix_world
    p0_w = mat @ p1.co
    p1_w = mat @ p1.handle_right
    p2_w = mat @ p2.handle_left
    p3_w = mat @ p2.co

    return evaluate_bezier(p0_w, p1_w, p2_w, p3_w, t)


def find_target_rail_object(target_name=None):
    """対象となるレール（CURVE）オブジェクトを検索"""
    if target_name and target_name in bpy.data.objects:
        obj = bpy.data.objects[target_name]
        if obj.type == 'CURVE':
            return obj

    # 1. "StageRail" を優先探索
    if "StageRail" in bpy.data.objects and bpy.data.objects["StageRail"].type == 'CURVE':
        return bpy.data.objects["StageRail"]

    # 2. object_type == "STAGE_RAIL" のCURVE
    for obj in bpy.data.objects:
        if obj.type == 'CURVE' and obj.get("object_type") == "STAGE_RAIL":
            return obj

    # 3. シーン内の最初のCURVE
    for obj in bpy.data.objects:
        if obj.type == 'CURVE':
            return obj

    return None


def find_stage_rail_object():
    """ステージ進行レール（StageRail）を検索"""
    if "StageRail" in bpy.data.objects and bpy.data.objects["StageRail"].type == 'CURVE':
        return bpy.data.objects["StageRail"]
    for obj in bpy.data.objects:
        if obj.type == 'CURVE' and obj.get("object_type") == "STAGE_RAIL":
            return obj
    return None


def find_camera_rail_object():
    """カメラ追従レール（CameraRail）を検索"""
    if "CameraRail" in bpy.data.objects and bpy.data.objects["CameraRail"].type == 'CURVE':
        return bpy.data.objects["CameraRail"]
    for obj in bpy.data.objects:
        if obj.type == 'CURVE' and obj.get("object_type") == "CAMERA_RAIL":
            return obj
    return None


def apply_rail_position_to_object(obj, target_rail=None):
    """
    オブジェクトの rail_pos = [t, y] プロパティに基づいて、
    レール上の計算座標（+高さオフセット）にオブジェクトの位置をスナップ配置
    """
    if "rail_pos" not in obj:
        return False

    rp = obj["rail_pos"]
    try:
        t = float(rp[0])
        height_offset = float(rp[1]) if len(rp) > 1 else 0.0
    except (ValueError, TypeError, IndexError):
        return False

    rail = target_rail or find_target_rail_object(obj.get("rail_target"))
    if not rail:
        return False

    rail_world_pos = get_rail_world_position(rail, t)
    if rail_world_pos is None:
        return False

    # BlenderではゲームのY軸（高さ）はZ軸
    final_world_pos = mathutils.Vector((
        rail_world_pos.x,
        rail_world_pos.y,
        rail_world_pos.z + height_offset
    ))

    if obj.parent:
        obj.location = obj.parent.matrix_world.inverted() @ final_world_pos
    else:
        obj.location = final_world_pos

    return True


def snap_all_objects_to_rail():
    """シーン内の rail_pos を持つ全オブジェクトをレール上に配置"""
    count = 0
    for obj in bpy.data.objects:
        if "rail_pos" in obj:
            if apply_rail_position_to_object(obj):
                count += 1
    return count


def find_closest_rail_t(curve_obj, target_world_pos, samples_per_segment=20):
    """任意のワールド座標から最寄りのレール進行度 t を逆算"""
    if not curve_obj or curve_obj.type != 'CURVE':
        return None
    curve_data = curve_obj.data
    if not curve_data.splines:
        return None
    spline = curve_data.splines[0]
    points = spline.bezier_points
    num_pts = len(points)
    if num_pts < 2:
        return 0.0

    is_loop = spline.use_cyclic_u
    max_segments = num_pts if is_loop else num_pts - 1

    best_dist_sq = float('inf')
    best_t = 0.0

    # 1. 粗いサンプリング探索
    for seg in range(max_segments):
        for s in range(samples_per_segment):
            local_t = s / float(samples_per_segment)
            t_cand = seg + local_t
            pos = get_rail_world_position(curve_obj, t_cand)
            if pos:
                dist_sq = (pos - target_world_pos).length_squared
                if dist_sq < best_dist_sq:
                    best_dist_sq = dist_sq
                    best_t = t_cand

    # 2. 局所探索で精度を高める
    step = 1.0 / (samples_per_segment * 2.0)
    for _ in range(8):
        t_left = best_t - step
        t_right = best_t + step
        pos_left = get_rail_world_position(curve_obj, t_left)
        pos_right = get_rail_world_position(curve_obj, t_right)
        d_left = (pos_left - target_world_pos).length_squared if pos_left else float('inf')
        d_right = (pos_right - target_world_pos).length_squared if pos_right else float('inf')
        if d_left < best_dist_sq:
            best_dist_sq = d_left
            best_t = t_left
        elif d_right < best_dist_sq:
            best_dist_sq = d_right
            best_t = t_right
        step *= 0.5

    return best_t


# ==========================================
# 2. カメラレール自動連携・オフセット生成ロジック
# ==========================================

def sync_camera_rail_from_stage(stage_rail, distance=25.0, height=5.0, flip_side=False):
    """
    StageRail のカーブ形状に基づき、外側オフセット（距離 & 高さ）を持った
    CameraRail を自動生成または同期更新する
    """
    if not stage_rail or stage_rail.type != 'CURVE':
        return None

    s_data = stage_rail.data
    if not s_data.splines:
        return None
    s_spline = s_data.splines[0]
    s_points = s_spline.bezier_points
    num_pts = len(s_points)
    if num_pts < 2:
        return None

    is_cyclic = s_spline.use_cyclic_u

    # CameraRail オブジェクトの取得または新規作成
    cam_obj = find_camera_rail_object()
    if not cam_obj:
        cam_curve = bpy.data.curves.new(name="CameraRail", type='CURVE')
        cam_curve.dimensions = '3D'
        cam_curve.resolution_u = s_data.resolution_u
        cam_obj = bpy.data.objects.new("CameraRail", cam_curve)
        cam_obj["object_type"] = "CAMERA_RAIL"
        col = stage_rail.users_collection[0] if stage_rail.users_collection else bpy.context.collection
        col.objects.link(cam_obj)
        print("[RailSync] 新規 CameraRail を作成しました")

    c_data = cam_obj.data
    if not c_data.splines:
        c_spline = c_data.splines.new('BEZIER')
    else:
        c_spline = c_data.splines[0]

    # スプライン点数を StageRail に合わせる
    c_pts = c_spline.bezier_points
    diff = num_pts - len(c_pts)
    if diff > 0:
        c_pts.add(diff)
    elif diff < 0:
        # 点数が多い場合はスプラインを再生成して合わせる
        c_data.splines.clear()
        c_spline = c_data.splines.new('BEZIER')
        c_pts = c_spline.bezier_points
        c_pts.add(num_pts - 1)

    c_spline.use_cyclic_u = is_cyclic
    cam_obj["loop"] = is_cyclic
    cam_obj["object_type"] = "CAMERA_RAIL"
    cam_obj.matrix_world = stage_rail.matrix_world.copy()

    # 全制御点の重心（X-Y中心）を計算して内外判定に使用
    center_xy = mathutils.Vector((0.0, 0.0, 0.0))
    for p in s_points:
        center_xy.x += p.co.x
        center_xy.y += p.co.y
    center_xy.x /= num_pts
    center_xy.y /= num_pts

    side_sign = -1.0 if flip_side else 1.0

    # 各制御点の法線オフセット計算
    for i in range(num_pts):
        sp = s_points[i]
        cp = c_pts[i]

        co = sp.co
        hl = sp.handle_left
        hr = sp.handle_right

        # 進行方向の接線ベクトル T
        tangent = (hr - hl).copy()
        tangent.z = 0.0  # 水平基準
        if tangent.length_squared < 1e-6:
            prev_p = s_points[(i - 1 + num_pts) % num_pts].co
            next_p = s_points[(i + 1) % num_pts].co
            tangent = (next_p - prev_p).copy()
            tangent.z = 0.0

        if tangent.length_squared > 1e-6:
            tangent.normalize()
        else:
            tangent = mathutils.Vector((1.0, 0.0, 0.0))

        # 水平法線 N (反時計回りに90度)
        normal = mathutils.Vector((-tangent.y, tangent.x, 0.0))

        # 重心から外側を向くように法線の符号を自動補正
        from_center = mathutils.Vector((co.x - center_xy.x, co.y - center_xy.y, 0.0))
        if from_center.dot(normal) < 0.0:
            normal = -normal

        normal *= side_sign

        # 新しい制御点の位置
        new_co = co + normal * distance
        new_co.z += height

        # ハンドルのスケーリング (外周への拡大に伴う曲率比率)
        radius_est = from_center.length
        scale_h = 1.0 + (distance / max(radius_est, 10.0)) if radius_est > 0.1 else 1.0
        scale_h = max(0.5, min(scale_h, 3.0))

        new_hl = new_co + (hl - co) * scale_h
        new_hr = new_co + (hr - co) * scale_h

        cp.co = new_co
        cp.handle_left = new_hl
        cp.handle_right = new_hr
        cp.handle_left_type = sp.handle_left_type
        cp.handle_right_type = sp.handle_right_type

    c_data.update()
    return cam_obj


# ==========================================
# 3. リアルタイムハンドラー群
# ==========================================

_is_magnet_snapping = False
_is_syncing_camera_rail = False

@bpy.app.handlers.persistent
def rail_magnet_snap_handler(scene, depsgraph):
    """オブジェクト移動時にレールが近ければ自動で吸着するハンドラー"""
    global _is_magnet_snapping
    if _is_magnet_snapping:
        return
    if not getattr(scene, "rail_magnet_snap", False):
        return

    try:
        active_obj = bpy.context.view_layer.objects.active
    except Exception:
        active_obj = None

    if not active_obj or active_obj.mode != 'OBJECT':
        return

    obj_type = active_obj.get("object_type", "")
    is_rail_obj = ("rail_pos" in active_obj) or (obj_type in ["ENEMY", "PLAYER_SPAWN", "GOAL"])
    if not is_rail_obj:
        return

    updated = False
    for update in depsgraph.updates:
        if update.id.original == active_obj and update.is_updated_transform:
            updated = True
            break
    if not updated:
        return

    rail = find_target_rail_object(active_obj.get("rail_target"))
    if not rail or rail == active_obj:
        return

    world_pos = active_obj.matrix_world.translation.copy()
    best_t = find_closest_rail_t(rail, world_pos, samples_per_segment=15)
    if best_t is None:
        return

    rail_pt = get_rail_world_position(rail, best_t)
    if rail_pt is None:
        return

    threshold = getattr(scene, "rail_magnet_distance", 3.0)
    h_dist_sq = (world_pos.x - rail_pt.x)**2 + (world_pos.y - rail_pt.y)**2

    if h_dist_sq <= (threshold * threshold):
        _is_magnet_snapping = True
        try:
            keep_height = getattr(scene, "rail_magnet_keep_height", True)
            if keep_height:
                final_z = world_pos.z
                height_offset = world_pos.z - rail_pt.z
            else:
                final_z = rail_pt.z
                height_offset = 0.0

            target_world_pos = mathutils.Vector((rail_pt.x, rail_pt.y, final_z))

            if active_obj.parent:
                active_obj.location = active_obj.parent.matrix_world.inverted() @ target_world_pos
            else:
                active_obj.location = target_world_pos

            active_obj["rail_pos"] = [round(best_t, 3), round(height_offset, 3)]
        finally:
            _is_magnet_snapping = False


@bpy.app.handlers.persistent
def camera_rail_sync_handler(scene, depsgraph):
    """StageRail の編集時に CameraRail を自動同期するハンドラー"""
    global _is_syncing_camera_rail
    if _is_syncing_camera_rail:
        return
    if not getattr(scene, "rail_camera_auto_sync", False):
        return

    stage_rail = find_stage_rail_object()
    if not stage_rail:
        return

    # StageRail の更新をチェック
    stage_updated = False
    for update in depsgraph.updates:
        if update.id.original == stage_rail:
            stage_updated = True
            break
    if not stage_updated:
        return

    _is_syncing_camera_rail = True
    try:
        dist = getattr(scene, "rail_camera_distance", 25.0)
        h = getattr(scene, "rail_camera_height", 5.0)
        flip = getattr(scene, "rail_camera_flip_side", False)
        sync_camera_rail_from_stage(stage_rail, distance=dist, height=h, flip_side=flip)
    finally:
        _is_syncing_camera_rail = False


# ==========================================
# 4. 接続点・球体ワイヤーフレーム 3Dビュー描画
# ==========================================

class DrawRailConnectionPoints:
    """StageRail & CameraRail の制御点（接続点）を球体ワイヤーフレームで描画"""
    handle = None

    @staticmethod
    def _create_wire_sphere_mesh(radius=0.8, segments=16):
        """3軸の大円リングからなる球体ワイヤーフレーム頂点群を生成"""
        verts = []
        indices = []

        # 1. XY 平面円 (Z=0)
        start = len(verts)
        for i in range(segments):
            angle = 2.0 * math.pi * (i / segments)
            verts.append((radius * math.cos(angle), radius * math.sin(angle), 0.0))
            indices.append((start + i, start + ((i + 1) % segments)))

        # 2. XZ 平面円 (Y=0)
        start = len(verts)
        for i in range(segments):
            angle = 2.0 * math.pi * (i / segments)
            verts.append((radius * math.cos(angle), 0.0, radius * math.sin(angle)))
            indices.append((start + i, start + ((i + 1) % segments)))

        # 3. YZ 平面円 (X=0)
        start = len(verts)
        for i in range(segments):
            angle = 2.0 * math.pi * (i / segments)
            verts.append((0.0, radius * math.cos(angle), radius * math.sin(angle)))
            indices.append((start + i, start + ((i + 1) % segments)))

        return verts, indices

    @staticmethod
    def draw():
        scene = bpy.context.scene
        if not getattr(scene, "rail_draw_wire_sphere", True):
            return

        radius = getattr(scene, "rail_wire_sphere_radius", 0.8)
        base_verts, base_indices = DrawRailConnectionPoints._create_wire_sphere_mesh(radius, segments=16)

        stage_rail = find_stage_rail_object()
        camera_rail = find_camera_rail_object()

        # 1. StageRail 接続点の球体ワイヤー描画 (シアン色)
        if stage_rail and stage_rail.type == 'CURVE' and stage_rail.data.splines:
            s_mat = stage_rail.matrix_world
            s_pts = stage_rail.data.splines[0].bezier_points
            s_world_pts = [s_mat @ p.co for p in s_pts]

            DrawRailConnectionPoints._render_spheres(s_world_pts, base_verts, base_indices, color=(0.1, 0.9, 1.0, 0.9))

            # 2. CameraRail 接続点の球体ワイヤー描画 (オレンジ色)
            if camera_rail and camera_rail.type == 'CURVE' and camera_rail.data.splines:
                c_mat = camera_rail.matrix_world
                c_pts = camera_rail.data.splines[0].bezier_points
                c_world_pts = [c_mat @ p.co for p in c_pts]

                DrawRailConnectionPoints._render_spheres(c_world_pts, base_verts, base_indices, color=(1.0, 0.7, 0.1, 0.9))

                # 3. 対応する接続点同士を結ぶ連携ライン描画 (半透明イエロー)
                pair_count = min(len(s_world_pts), len(c_world_pts))
                if pair_count > 0:
                    link_verts = []
                    link_indices = []
                    for i in range(pair_count):
                        p1 = s_world_pts[i]
                        p2 = c_world_pts[i]
                        idx = len(link_verts)
                        link_verts.append((p1.x, p1.y, p1.z))
                        link_verts.append((p2.x, p2.y, p2.z))
                        link_indices.append((idx, idx + 1))

                    shader = gpu.shader.from_builtin('UNIFORM_COLOR')
                    batch = gpu_extras.batch.batch_for_shader(
                        shader, 'LINES', {"pos": link_verts}, indices=link_indices
                    )
                    shader.bind()
                    shader.uniform_float("color", (1.0, 1.0, 0.3, 0.5))
                    gpu.state.line_width_set(1.5)
                    batch.draw(shader)

    @staticmethod
    def _render_spheres(center_positions, base_verts, base_indices, color):
        """複数の中心座標に球体ワイヤーフレームを一括バッチ描画"""
        if not center_positions:
            return

        all_verts = []
        all_indices = []

        for center in center_positions:
            v_start = len(all_verts)
            for bv in base_verts:
                all_verts.append((center.x + bv[0], center.y + bv[1], center.z + bv[2]))
            for bi in base_indices:
                all_indices.append((v_start + bi[0], v_start + bi[1]))

        shader = gpu.shader.from_builtin('UNIFORM_COLOR')
        batch = gpu_extras.batch.batch_for_shader(
            shader, 'LINES', {"pos": all_verts}, indices=all_indices
        )
        shader.bind()
        shader.uniform_float("color", color)
        gpu.state.line_width_set(1.8)
        batch.draw(shader)


# ==========================================
# 5. オペレーター群
# ==========================================

class MYADDON_OT_snap_to_rail(bpy.types.Operator):
    bl_idname = "myaddon.snap_to_rail"
    bl_label = "レール位置にスナップ"
    bl_description = "設定された rail_pos [t, y] のレール上の位置へ配置します"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        obj = context.object
        if not obj or "rail_pos" not in obj:
            self.report({"WARNING"}, "オブジェクトに rail_pos が設定されていません")
            return {"CANCELLED"}
        if apply_rail_position_to_object(obj):
            self.report({"INFO"}, f"'{obj.name}' をレール上に配置しました")
            return {"FINISHED"}
        else:
            self.report({"WARNING"}, "レールが見つからないか、配置に失敗しました")
            return {"CANCELLED"}


class MYADDON_OT_calculate_rail_pos(bpy.types.Operator):
    bl_idname = "myaddon.calculate_rail_pos"
    bl_label = "現在位置から rail_pos を逆算"
    bl_description = "現在の3D位置から最も近いレール上の進行度 t と高さ y を計算して rail_pos を設定します"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        obj = context.object
        if not obj:
            return {"CANCELLED"}
        rail = find_target_rail_object(obj.get("rail_target"))
        if not rail:
            self.report({"WARNING"}, "レールオブジェクトが見つかりません")
            return {"CANCELLED"}

        world_pos = obj.matrix_world.translation.copy()
        best_t = find_closest_rail_t(rail, world_pos)
        if best_t is None:
            self.report({"WARNING"}, "レール位置の計算に失敗しました")
            return {"CANCELLED"}

        rail_pt = get_rail_world_position(rail, best_t)
        height_offset = world_pos.z - rail_pt.z

        obj["rail_pos"] = [round(best_t, 3), round(height_offset, 3)]
        apply_rail_position_to_object(obj, rail)
        self.report({"INFO"}, f"rail_pos を更新しました: t={round(best_t, 3)}, y={round(height_offset, 3)}")
        return {"FINISHED"}


class MYADDON_OT_add_rail_pos(bpy.types.Operator):
    bl_idname = "myaddon.add_rail_pos"
    bl_label = "rail_pos プロパティを追加"
    bl_description = "['rail_pos'] カスタムプロパティを追加し、最寄りレールに配置します"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        obj = context.object
        if not obj:
            return {"CANCELLED"}
        obj["rail_pos"] = [0.0, 0.0]
        bpy.ops.myaddon.calculate_rail_pos()
        return {"FINISHED"}


class MYADDON_OT_sync_camera_rail(bpy.types.Operator):
    bl_idname = "myaddon.sync_camera_rail"
    bl_label = "StageRail から CameraRail を生成・同期"
    bl_description = "StageRail の形状・制御点から指定の距離・高さで CameraRail を自動生成/同期更新します"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        scene = context.scene
        stage_rail = find_stage_rail_object()
        if not stage_rail:
            self.report({"WARNING"}, "StageRail が見つかりません。まず 'StageRail' を作成してください。")
            return {"CANCELLED"}

        dist = getattr(scene, "rail_camera_distance", 25.0)
        h = getattr(scene, "rail_camera_height", 5.0)
        flip = getattr(scene, "rail_camera_flip_side", False)

        cam_obj = sync_camera_rail_from_stage(stage_rail, distance=dist, height=h, flip_side=flip)
        if cam_obj:
            self.report({"INFO"}, f"CameraRail を同期しました (外側距離={dist}m, 高さ={h}m)")
            return {"FINISHED"}
        else:
            self.report({"WARNING"}, "CameraRail の同期に失敗しました")
            return {"CANCELLED"}


class MYADDON_OT_unlink_camera_rail(bpy.types.Operator):
    bl_idname = "myaddon.unlink_camera_rail"
    bl_label = "連携を分離（独立編集モード）"
    bl_description = "StageRail と CameraRail の自動追従を解除し、カメラレールを個別に自由編集できるようにします"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        context.scene.rail_camera_auto_sync = False
        self.report({"INFO"}, "CameraRail の自動連携を解除しました。個別に編集可能です。")
        return {"FINISHED"}


# ==========================================
# 6. パネル
# ==========================================

class OBJECT_PT_rail_settings(bpy.types.Panel):
    """レール進行度設定 & カメラレール自動連携パネル"""
    bl_idname = "OBJECT_PT_rail_settings"
    bl_label = "Rail Settings & Camera Sync"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "object"

    def draw(self, context):
        layout = self.layout
        obj = context.object
        scene = context.scene
        if not obj:
            return

        stage_rail = find_stage_rail_object()
        camera_rail = find_camera_rail_object()

        # ── 1. レール連携ステータス ──
        box_link = layout.box()
        box_link.label(text="📸 カメラ・ステージレール自動連携:", icon='CAMERA_DATA')

        row = box_link.row(align=True)
        is_sync = getattr(scene, "rail_camera_auto_sync", False)
        if is_sync:
            row.label(text="状態: 🟢 リアルタイム連携中", icon='LINKED')
        else:
            row.label(text="状態: ⚪ 分離中 (個別編集モード)", icon='UNLINKED')

        col = box_link.column(align=True)
        col.prop(scene, "rail_camera_distance", text="カメラ距離 (m)")
        col.prop(scene, "rail_camera_height", text="カメラ高さ (m)")
        col.prop(scene, "rail_camera_flip_side", text="オフセット向きを反転 (内側)")

        row = box_link.row(align=True)
        row.operator(MYADDON_OT_sync_camera_rail.bl_idname, text="生成・同期", icon='FILE_REFRESH')
        if is_sync:
            row.operator(MYADDON_OT_unlink_camera_rail.bl_idname, text="分離する", icon='UNLINKED')
        else:
            row.prop(scene, "rail_camera_auto_sync", text="リアルタイム連携", icon='LINKED', toggle=True)

        # ── 2. 接続点ワイヤーフレーム表示設定 ──
        box_wire = layout.box()
        box_wire.label(text="🔮 接続点の球体ワイヤーフレーム表示:", icon='SHADING_WIRE')
        row = box_wire.row()
        row.prop(scene, "rail_draw_wire_sphere", text="球体ワイヤー表示")
        if scene.rail_draw_wire_sphere:
            box_wire.prop(scene, "rail_wire_sphere_radius", text="球体サイズ (m)")
            row_info = box_wire.row(align=True)
            row_info.label(text="🔵 StageRail (シアン)  /  🟠 CameraRail (オレンジ)")

        layout.separator()

        # ── 3. レール自動マグネット吸着 ──
        rail = find_target_rail_object(obj.get("rail_target"))
        rail_name = rail.name if rail else "未検出"
        row = layout.row()
        row.label(text=f"参照レール: {rail_name}", icon='CURVE_DATA')

        box_magnet = layout.box()
        box_magnet.label(text="🧲 レール自動吸着 (近づくと吸着):", icon='SNAP_ON')
        row = box_magnet.row()
        row.prop(scene, "rail_magnet_snap", text="自動吸着モード")
        if scene.rail_magnet_snap:
            box_magnet.prop(scene, "rail_magnet_distance", text="吸着距離 (m)")
            box_magnet.prop(scene, "rail_magnet_keep_height", text="現在の高さを維持")

        layout.separator()

        # ── 4. rail_pos 配置設定 ──
        if "rail_pos" in obj:
            rp = obj["rail_pos"]
            box = layout.box()
            try:
                t_val = rp[0]
                y_val = rp[1] if len(rp) > 1 else 0.0
                box.label(text=f"進行度 t: {t_val:.3f} | 高さ y: {y_val:.3f}")
            except Exception:
                box.label(text=f"rail_pos: {rp}")

            row = layout.row(align=True)
            row.operator(MYADDON_OT_snap_to_rail.bl_idname, text="レール位置へスナップ", icon='SNAP_ON')
            row.operator(MYADDON_OT_calculate_rail_pos.bl_idname, text="現在地から逆算", icon='EYEDROPPER')
        else:
            layout.operator(MYADDON_OT_add_rail_pos.bl_idname, text="rail_pos を追加してスナップ", icon='ADD')


# ==========================================
# 7. 登録・解除
# ==========================================

classes = (
    MYADDON_OT_snap_to_rail,
    MYADDON_OT_calculate_rail_pos,
    MYADDON_OT_add_rail_pos,
    MYADDON_OT_sync_camera_rail,
    MYADDON_OT_unlink_camera_rail,
    OBJECT_PT_rail_settings,
)

def register_rail_snap():
    # レール自動吸着プロパティ
    bpy.types.Scene.rail_magnet_snap = bpy.props.BoolProperty(
        name="レール自動吸着",
        description="Emptyオブジェクトの移動時にレールが近ければ自動で吸着します",
        default=True,
    )
    bpy.types.Scene.rail_magnet_distance = bpy.props.FloatProperty(
        name="吸着距離",
        description="この距離（メートル）以内に近づくとレールに自動吸着します",
        default=3.0,
        min=0.1,
        max=50.0,
    )
    bpy.types.Scene.rail_magnet_keep_height = bpy.props.BoolProperty(
        name="現在の高さを維持",
        description="吸着時にオブジェクトのZ座標（高さ）を維持し、高さオフセットとして記録します",
        default=True,
    )

    # カメラレール連携プロパティ
    bpy.types.Scene.rail_camera_auto_sync = bpy.props.BoolProperty(
        name="リアルタイム自動連携",
        description="StageRail 編集時に CameraRail を自動同期します",
        default=False,
    )
    bpy.types.Scene.rail_camera_distance = bpy.props.FloatProperty(
        name="カメラレール距離",
        description="StageRail からの外側オフセット距離（メートル）",
        default=25.0,
        min=1.0,
        max=200.0,
    )
    bpy.types.Scene.rail_camera_height = bpy.props.FloatProperty(
        name="カメラレール高さ",
        description="StageRail からの高さオフセット（Z軸、メートル）",
        default=5.0,
        min=-50.0,
        max=100.0,
    )
    bpy.types.Scene.rail_camera_flip_side = bpy.props.BoolProperty(
        name="オフセット向き反転",
        description="オフセット方向を反転（内側向き）にします",
        default=False,
    )

    # 接続点・球体ワイヤー表示プロパティ
    bpy.types.Scene.rail_draw_wire_sphere = bpy.props.BoolProperty(
        name="接続点の球体ワイヤー表示",
        description="StageRail と CameraRail の制御点（接続点）に球体ワイヤーフレームを描画します",
        default=True,
    )
    bpy.types.Scene.rail_wire_sphere_radius = bpy.props.FloatProperty(
        name="球体サイズ",
        description="接続点に描画する球体ワイヤーフレームの半径（メートル）",
        default=0.8,
        min=0.1,
        max=10.0,
    )

    # ハンドラー登録
    if rail_magnet_snap_handler not in bpy.app.handlers.depsgraph_update_post:
        bpy.app.handlers.depsgraph_update_post.append(rail_magnet_snap_handler)

    if camera_rail_sync_handler not in bpy.app.handlers.depsgraph_update_post:
        bpy.app.handlers.depsgraph_update_post.append(camera_rail_sync_handler)

    if DrawRailConnectionPoints.handle is None:
        DrawRailConnectionPoints.handle = bpy.types.SpaceView3D.draw_handler_add(
            DrawRailConnectionPoints.draw, (), "WINDOW", "POST_VIEW"
        )


def unregister_rail_snap():
    if DrawRailConnectionPoints.handle:
        bpy.types.SpaceView3D.draw_handler_remove(DrawRailConnectionPoints.handle, "WINDOW")
        DrawRailConnectionPoints.handle = None

    if camera_rail_sync_handler in bpy.app.handlers.depsgraph_update_post:
        bpy.app.handlers.depsgraph_update_post.remove(camera_rail_sync_handler)

    if rail_magnet_snap_handler in bpy.app.handlers.depsgraph_update_post:
        bpy.app.handlers.depsgraph_update_post.remove(rail_magnet_snap_handler)

    for prop in (
        "rail_magnet_snap", "rail_magnet_distance", "rail_magnet_keep_height",
        "rail_camera_auto_sync", "rail_camera_distance", "rail_camera_height", "rail_camera_flip_side",
        "rail_draw_wire_sphere", "rail_wire_sphere_radius",
    ):
        if hasattr(bpy.types.Scene, prop):
            delattr(bpy.types.Scene, prop)
