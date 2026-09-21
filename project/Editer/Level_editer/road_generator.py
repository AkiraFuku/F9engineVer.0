import bpy
import math
import mathutils
import os

# ==========================================
# レール沿い道路自動生成モジュール (road_generator.py)
#
# StageRail（または選択中のカーブ）に沿って、
# 幅・厚み・ガードレール（側壁）付きの3D道路メッシュを自動生成します。
# 生成されたメッシュは OBJ として保存され、
# ゲーム側（C++）の StageBlock で自動的にコリジョン対応の足場になります。
# ==========================================

ROAD_OBJECT_NAME = "Road_StageRail"
DEFAULT_ROAD_OBJ_REL = "resources/Stagemap/road_generated.obj"
DEFAULT_TEXTURE_REL  = "resources/Stagemap/863603.png"


def _get_project_root():
    """プロジェクトのルートディレクトリパスを取得"""
    cur_file = os.path.abspath(__file__)
    # Editer/Level_editer -> Editer -> project
    editer_dir = os.path.dirname(os.path.dirname(cur_file))
    return os.path.dirname(editer_dir)


def _evaluate_bezier(p0, p1, p2, p3, t):
    """3次ベジェ曲線の補間点"""
    u = 1.0 - t
    return (u**3) * p0 + 3.0*(u**2)*t*p1 + 3.0*u*(t**2)*p2 + (t**3)*p3


def _evaluate_bezier_tangent(p0, p1, p2, p3, t):
    """3次ベジェ曲線の接線ベクトル（速度ベクトル）"""
    u = 1.0 - t
    return 3.0*(u**2)*(p1 - p0) + 6.0*u*t*(p2 - p1) + 3.0*(t**2)*(p3 - p2)


def sample_curve_frenet(curve_obj, step_distance=0.5):
    """
    カーブをステップ距離（約 step_distance メートル間隔）でサンプリングし、
    各点のワールド座標、接線（Tangent）、幅方向ベクトル（Binormal）、法線（Normal）を返す。
    returns: list of dict(pos, tangent, binormal, normal, dist)
    """
    if not curve_obj or curve_obj.type != 'CURVE':
        return []
    if not curve_obj.data.splines:
        return []

    spline = curve_obj.data.splines[0]
    mat = curve_obj.matrix_world
    is_cyclic = spline.use_cyclic_u

    raw_points = []

    if spline.type == 'BEZIER':
        bp = spline.bezier_points
        num_bp = len(bp)
        if num_bp < 2:
            return []
        num_segs = num_bp if is_cyclic else (num_bp - 1)

        # セグメントごとに長さを概算してサンプリング数を決定
        for seg in range(num_segs):
            i0 = seg
            i1 = (seg + 1) % num_bp
            p0 = mat @ bp[i0].co
            p1 = mat @ bp[i0].handle_right
            p2 = mat @ bp[i1].handle_left
            p3 = mat @ bp[i1].co

            # 近似長さ
            chord = (p3 - p0).length
            ctrl_poly = (p1 - p0).length + (p2 - p1).length + (p3 - p2).length
            seg_len = (chord + ctrl_poly) * 0.5
            subdivs = max(int(seg_len / max(step_distance, 0.1)), 2)

            for s in range(subdivs):
                if seg > 0 and s == 0:
                    continue  # 重複スキップ
                t = float(s) / float(subdivs)
                pos = _evaluate_bezier(p0, p1, p2, p3, t)
                tan = _evaluate_bezier_tangent(p0, p1, p2, p3, t)
                if tan.length > 1e-5:
                    tan = tan.normalized()
                else:
                    tan = mathutils.Vector((0, 1, 0))
                raw_points.append((pos, tan))

        # 終端点（非ループ時）
        if not is_cyclic:
            p0 = mat @ bp[num_bp - 2].co
            p1 = mat @ bp[num_bp - 2].handle_right
            p2 = mat @ bp[num_bp - 1].handle_left
            p3 = mat @ bp[num_bp - 1].co
            pos = p3
            tan = _evaluate_bezier_tangent(p0, p1, p2, p3, 1.0)
            if tan.length > 1e-5:
                tan = tan.normalized()
            else:
                tan = (p3 - p2).normalized() if (p3 - p2).length > 1e-5 else mathutils.Vector((0, 1, 0))
            raw_points.append((pos, tan))

    else:
        # POLY or NURBS
        pts = spline.points
        num_pts = len(pts)
        if num_pts < 2:
            return []
        for i in range(num_pts):
            p = mat @ pts[i].co.xyz
            raw_points.append((p, mathutils.Vector((0, 1, 0))))

        # 接線計算
        for i in range(len(raw_points)):
            p = raw_points[i][0]
            if i == 0:
                p_next = raw_points[1][0]
                tan = (p_next - p).normalized()
            elif i == len(raw_points) - 1:
                p_prev = raw_points[i - 1][0]
                tan = (p - p_prev).normalized()
            else:
                p_prev = raw_points[i - 1][0]
                p_next = raw_points[i + 1][0]
                tan = (p_next - p_prev).normalized()
            raw_points[i] = (p, tan)

    if len(raw_points) < 2:
        return []

    # 各サンプルのフレーム（Binormal, Normal）および累積距離を計算
    samples = []
    accum_dist = 0.0
    global_up = mathutils.Vector((0, 0, 1))

    for i, (pos, tan) in enumerate(raw_points):
        if i > 0:
            accum_dist += (pos - raw_points[i - 1][0]).length

        # 接線とグローバル上ベクトルから水平幅方向（Binormal）を算出
        if abs(tan.dot(global_up)) > 0.98:
            temp_side = mathutils.Vector((1, 0, 0))
            binormal = tan.cross(temp_side).normalized()
        else:
            binormal = tan.cross(global_up).normalized()

        # 上法線
        normal = binormal.cross(tan).normalized()
        if normal.z < 0:
            normal = -normal
            binormal = -binormal

        samples.append({
            "pos": pos,
            "tangent": tan,
            "binormal": binormal,
            "normal": normal,
            "dist": accum_dist,
        })

    return samples


def build_road_geometry(
    samples,
    width=4.0,
    thickness=0.3,
    sidewall_height=0.0,
    sidewall_thickness=0.15,
    uv_tile_length=2.0,
):
    """
    サンプリング点列から道路の3D頂点、面、UV座標を構築する。
    戻り値: (verts, faces, uvs)
    """
    if len(samples) < 2:
        return [], [], []

    half_w = width * 0.5
    half_th = thickness
    sw_h = sidewall_height
    sw_w = sidewall_thickness

    verts = []
    uvs = []
    faces = []

    has_wall = (sw_h > 0.05)

    for s in samples:
        P = s["pos"]
        B = s["binormal"]
        N = s["normal"]

        if not has_wall:
            # 0: 上面左, 1: 上面右, 2: 底面右, 3: 底面左
            p_top_l = P - B * half_w
            p_top_r = P + B * half_w
            p_bot_r = p_top_r - N * half_th
            p_bot_l = p_top_l - N * half_th

            ring = [p_top_l, p_top_r, p_bot_r, p_bot_l]
            verts.extend(ring)
        else:
            # ガードレール付き (10頂点リング)
            p_road_l = P - B * half_w
            p_road_r = P + B * half_w
            p_bot_l  = p_road_l - N * half_th
            p_bot_r  = p_road_r - N * half_th

            p_wall_l_inner_top = p_road_l + N * sw_h
            p_wall_l_outer_top = p_road_l - B * sw_w + N * sw_h
            p_wall_l_outer_bot = p_road_l - B * sw_w - N * half_th

            p_wall_r_inner_top = p_road_r + N * sw_h
            p_wall_r_outer_top = p_road_r + B * sw_w + N * sw_h
            p_wall_r_outer_bot = p_road_r + B * sw_w - N * half_th

            ring = [
                p_wall_l_outer_bot,  # 0
                p_wall_l_outer_top,  # 1
                p_wall_l_inner_top,  # 2
                p_road_l,            # 3 (道路上面左)
                p_road_r,            # 4 (道路上面右)
                p_wall_r_inner_top,  # 5
                p_wall_r_outer_top,  # 6
                p_wall_r_outer_bot,  # 7
                p_bot_r,             # 8
                p_bot_l,             # 9
            ]
            verts.extend(ring)

    num_samples = len(samples)

    # クアッド面接続
    if not has_wall:
        for i in range(num_samples - 1):
            r0 = i * 4
            r1 = (i + 1) * 4
            v0 = samples[i]["dist"] / max(uv_tile_length, 0.1)
            v1 = samples[i + 1]["dist"] / max(uv_tile_length, 0.1)

            # 上面 (0 -> 1 -> r1_1 -> r1_0)
            faces.append((r0 + 0, r0 + 1, r1 + 1, r1 + 0))
            uvs.append([(0.0, v0), (1.0, v0), (1.0, v1), (0.0, v1)])

            # 右側面 (1 -> 2 -> r1_2 -> r1_1)
            faces.append((r0 + 1, r0 + 2, r1 + 2, r1 + 1))
            uvs.append([(1.0, v0), (1.1, v0), (1.1, v1), (1.0, v1)])

            # 底面 (2 -> 3 -> r1_3 -> r1_2)
            faces.append((r0 + 2, r0 + 3, r1 + 3, r1 + 2))
            uvs.append([(1.0, v0), (0.0, v0), (0.0, v1), (1.0, v1)])

            # 左側面 (3 -> 0 -> r1_0 -> r1_3)
            faces.append((r0 + 3, r0 + 0, r1 + 0, r1 + 3))
            uvs.append([(0.0, v0), (-0.1, v0), (-0.1, v1), (0.0, v1)])

        # 始端キャップ
        faces.append((0, 3, 2, 1))
        uvs.append([(0.0, 0.0), (0.0, 1.0), (1.0, 1.0), (1.0, 0.0)])
        # 終端キャップ
        last_r = (num_samples - 1) * 4
        faces.append((last_r + 0, last_r + 1, last_r + 2, last_r + 3))
        uvs.append([(0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0)])

    else:
        for i in range(num_samples - 1):
            r0 = i * 10
            r1 = (i + 1) * 10
            v0 = samples[i]["dist"] / max(uv_tile_length, 0.1)
            v1 = samples[i + 1]["dist"] / max(uv_tile_length, 0.1)

            for seg_idx in range(9):
                a = r0 + seg_idx
                b = r0 + seg_idx + 1
                c = r1 + seg_idx + 1
                d = r1 + seg_idx
                faces.append((a, b, c, d))
                u0 = float(seg_idx) / 9.0
                u1 = float(seg_idx + 1) / 9.0
                uvs.append([(u0, v0), (u1, v0), (u1, v1), (u0, v1)])

            faces.append((r0 + 9, r0 + 0, r1 + 0, r1 + 9))
            uvs.append([(1.0, v0), (0.0, v0), (0.0, v1), (1.0, v1)])

    vert_tuples = [(v.x, v.y, v.z) for v in verts]
    return vert_tuples, faces, uvs


def export_road_to_obj_file(filepath, verts, faces, uvs, convert_to_game_coords=True):
    """
    生成した道路メッシュをゲームエンジン対応の OBJ ファイルとして直接書き出す。
    """
    os.makedirs(os.path.dirname(filepath), exist_ok=True)

    with open(filepath, "w", encoding="utf-8") as f:
        f.write("# Generated Road Mesh by Level Editor\n")
        f.write("o RoadMesh\n")

        for vx, vy, vz in verts:
            if convert_to_game_coords:
                # Blender(X, Y奥, Z上) -> Game(X右, Y上, Z奥)
                f.write(f"v {vx:.4f} {vz:.4f} {vy:.4f}\n")
            else:
                f.write(f"v {vx:.4f} {vy:.4f} {vz:.4f}\n")

        for face_uv in uvs:
            for u, v in face_uv:
                f.write(f"vt {u:.4f} {v:.4f}\n")

        if convert_to_game_coords:
            f.write("vn 0.0 1.0 0.0\n")
        else:
            f.write("vn 0.0 0.0 1.0\n")

        uv_idx = 1
        for face in faces:
            f.write("f")
            for v_idx in face:
                f.write(f" {v_idx + 1}/{uv_idx}/1")
                uv_idx += 1
            f.write("\n")

    print(f"[RoadGen] OBJファイルを保存しました: {filepath} ({len(verts)} verts, {len(faces)} faces)")


def create_or_update_road_object(
    curve_obj,
    width=4.0,
    thickness=0.3,
    sidewall_height=0.0,
    uv_tile_length=2.0,
    texture_rel=DEFAULT_TEXTURE_REL,
    step_distance=0.5,
):
    """
    カーブから道路メッシュを生成し、Blenderシーンに配置およびOBJ保存する。
    """
    samples = sample_curve_frenet(curve_obj, step_distance=step_distance)
    if not samples or len(samples) < 2:
        return False, "カーブのサンプリングに失敗しました。制御点が2つ以上あるか確認してください。"

    verts, faces, uvs = build_road_geometry(
        samples,
        width=width,
        thickness=thickness,
        sidewall_height=sidewall_height,
        uv_tile_length=uv_tile_length,
    )

    if not verts or not faces:
        return False, "ジオメトリの構築に失敗しました。"

    root_dir = _get_project_root()
    obj_filename = "road_generated.obj"
    full_obj_path = os.path.join(root_dir, "resources", "Stagemap", obj_filename)

    # 1. ゲーム用OBJファイル書き出し
    try:
        export_road_to_obj_file(full_obj_path, verts, faces, uvs, convert_to_game_coords=True)
    except Exception as e:
        print(f"[RoadGen] OBJ書き出し警告: {e}")

    # 2. Blenderシーン内にメッシュオブジェクトを作成または更新
    mesh_name = f"Mesh_{ROAD_OBJECT_NAME}"
    if mesh_name in bpy.data.meshes:
        mesh = bpy.data.meshes[mesh_name]
        mesh.clear_geometry()
    else:
        mesh = bpy.data.meshes.new(mesh_name)

    mesh.from_pydata(verts, [], faces)

    uv_layer = mesh.uv_layers.new(name="UVMap")
    uv_flat = [uv for face_uv in uvs for uv in face_uv]
    for i, loop in enumerate(mesh.loops):
        if i < len(uv_flat):
            uv_layer.data[i].uv = uv_flat[i]

    mesh.update()

    # マテリアル設定
    if texture_rel:
        try:
            from .terrain_generator import _get_or_create_terrain_material
            mat = _get_or_create_terrain_material("Mat_RoadGenerated", texture_rel)
            if mat:
                if not mesh.materials:
                    mesh.materials.append(mat)
                else:
                    mesh.materials[0] = mat
        except Exception:
            pass

    if ROAD_OBJECT_NAME in bpy.data.objects:
        road_obj = bpy.data.objects[ROAD_OBJECT_NAME]
        road_obj.data = mesh
    else:
        road_obj = bpy.data.objects.new(ROAD_OBJECT_NAME, mesh)
        bpy.context.collection.objects.link(road_obj)

    road_obj.location = (0, 0, 0)
    road_obj.rotation_euler = (0, 0, 0)
    road_obj.scale = (1, 1, 1)

    # カスタムプロパティ設定（ゲーム側で BLOCK として認識されるように）
    road_obj["object_type"] = "BLOCK"
    road_obj["file_name"] = obj_filename
    road_obj["model_dir"] = "resources/Stagemap"
    road_obj["texture"] = texture_rel

    return True, f"道路メッシュ '{ROAD_OBJECT_NAME}' を生成しました！（幅: {width:.1f}m, 長さ: {samples[-1]['dist']:.1f}m）"


class MYADDON_OT_generate_road_along_rail(bpy.types.Operator):
    bl_idname = "myaddon.generate_road_along_rail"
    bl_label = "レールに沿って道をつくる"
    bl_description = "選択中のレール（StageRail）に沿って、指定した幅・厚みの道路メッシュを自動生成します"
    bl_options = {"REGISTER", "UNDO"}

    road_width: bpy.props.FloatProperty(
        name="道路の幅 (m)",
        description="道路の横幅（メートル）",
        default=4.0,
        min=0.5,
        max=50.0,
    )
    road_thickness: bpy.props.FloatProperty(
        name="道路の厚み (m)",
        description="道路の厚み（メートル）",
        default=0.3,
        min=0.05,
        max=10.0,
    )
    sidewall_height: bpy.props.FloatProperty(
        name="ガードレール高さ (m)",
        description="左右の側壁/ガードレールの高さ（0で壁なし）",
        default=0.0,
        min=0.0,
        max=5.0,
    )
    uv_tile_length: bpy.props.FloatProperty(
        name="UVタイル間隔 (m)",
        description="テクスチャが1周する道路の長さ",
        default=2.0,
        min=0.5,
        max=50.0,
    )
    step_distance: bpy.props.FloatProperty(
        name="サンプリング間隔 (m)",
        description="ポリゴンの分割間隔（小さいほど滑らか）",
        default=0.5,
        min=0.1,
        max=5.0,
    )

    @classmethod
    def poll(cls, context):
        obj = context.object
        if obj and obj.type == 'CURVE':
            return True
        return "StageRail" in bpy.data.objects

    def invoke(self, context, event):
        return context.window_manager.invoke_props_dialog(self, width=320)

    def draw(self, context):
        layout = self.layout
        layout.label(text="=== 道路生成パラメータ ===", icon="ROAD")
        col = layout.column(align=True)
        col.prop(self, "road_width")
        col.prop(self, "road_thickness")
        col.prop(self, "sidewall_height")
        layout.separator()
        col2 = layout.column(align=True)
        col2.prop(self, "step_distance")
        col2.prop(self, "uv_tile_length")

    def execute(self, context):
        obj = context.object
        if not obj or obj.type != 'CURVE':
            if "StageRail" in bpy.data.objects:
                obj = bpy.data.objects["StageRail"]
            else:
                self.report({'ERROR'}, "カーブオブジェクト（レール）を選択してください。")
                return {'CANCELLED'}

        ok, msg = create_or_update_road_object(
            curve_obj=obj,
            width=self.road_width,
            thickness=self.road_thickness,
            sidewall_height=self.sidewall_height,
            uv_tile_length=self.uv_tile_length,
            step_distance=self.step_distance,
        )

        if not ok:
            self.report({'ERROR'}, msg)
            return {'CANCELLED'}

        self.report({'INFO'}, msg)
        return {'FINISHED'}
