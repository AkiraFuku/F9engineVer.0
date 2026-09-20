import bpy
import math
import mathutils
import os

# ==========================================
# コース地形自動生成モジュール（terrain_grid 版）
#
# StageRail のベジェ曲線コースに合わせて、
# ゲーム側の動的地形生成機能（terrain_grid）用の
# TERRAINオブジェクトをBlenderシーンに自動配置する。
#
# ゲーム側の動作:
#   file_name == "terrain_grid" の TERRAIN が読み込まれると
#   StageBlock::Initialize() が ModelManager::CreateTerrainModel() を呼び出し、
#   properties の size_x/size_y/divisions_x/divisions_y/uv_tile を使って
#   頂点データを動的生成する。
# ==========================================

_TERRAIN_OBJECT_NAME = "TerrainGround"


# ------------------------------------------
# ベジェ補間ユーティリティ
# ------------------------------------------

def _evaluate_bezier(p0, p1, p2, p3, t):
    """3次ベジェ曲線の補間点 (mathutils.Vector)"""
    u = 1.0 - t
    return (u**3) * p0 + 3.0*(u**2)*t*p1 + 3.0*u*(t**2)*p2 + (t**3)*p3


def _get_stage_rail():
    """シーン内の StageRail (STAGE_RAIL) カーブを取得"""
    for name in ("StageRail", "stageRail", "stage_rail"):
        if name in bpy.data.objects:
            obj = bpy.data.objects[name]
            if obj.type == "CURVE":
                return obj
    for obj in bpy.data.objects:
        if obj.type == "CURVE" and obj.get("object_type") == "STAGE_RAIL":
            return obj
    for obj in bpy.data.objects:
        if obj.type == "CURVE":
            return obj
    return None


def _sample_curve_points(curve_obj, num_samples=64):
    """
    カーブ上を num_samples 点でサンプリングしてワールド座標リストを返す。
    """
    spline = curve_obj.data.splines[0]
    points = spline.bezier_points
    num_pts = len(points)
    is_loop = spline.use_cyclic_u
    num_segs = num_pts if is_loop else (num_pts - 1)
    if num_segs <= 0:
        return []

    mat = curve_obj.matrix_world
    sampled = []

    for step in range(num_samples):
        global_t = (float(step) / float(num_samples)) * float(num_segs)

        if is_loop:
            global_t = global_t % float(num_segs)
        else:
            global_t = min(global_t, float(num_segs))

        idx1 = int(global_t)
        if is_loop:
            idx2 = (idx1 + 1) % num_pts
        else:
            idx1 = min(idx1, num_pts - 2)
            idx2 = idx1 + 1

        local_t = global_t - float(idx1)
        bp1 = points[idx1]
        bp2 = points[idx2]

        p0 = mat @ bp1.co
        p1 = mat @ bp1.handle_right
        p2 = mat @ bp2.handle_left
        p3 = mat @ bp2.co

        pos = _evaluate_bezier(p0, p1, p2, p3, local_t)
        sampled.append(pos)

    return sampled


def _compute_rail_bounds(sampled_points):
    """
    サンプリング点から XY（Blender XY = ゲームの XZ）の
    バウンディングボックス中心・サイズを計算して返す。
    returns: (center_x, center_y, span_x, span_y)
    """
    if not sampled_points:
        return (0.0, 0.0, 100.0, 100.0)

    xs = [p.x for p in sampled_points]
    ys = [p.y for p in sampled_points]

    min_x, max_x = min(xs), max(xs)
    min_y, max_y = min(ys), max(ys)

    cx = (min_x + max_x) * 0.5
    cy = (min_y + max_y) * 0.5
    sx = max_x - min_x
    sy = max_y - min_y

    return cx, cy, sx, sy


# ------------------------------------------
# Blender プレビューメッシュの生成
# ------------------------------------------

# ------------------------------------------
# テクスチャ・マテリアル ユーティリティ
# ------------------------------------------

def _resolve_texture_path(rel_or_abs_path):
    """テクスチャパスの絶対パスを解決"""
    if not rel_or_abs_path:
        return ""
    if os.path.isabs(rel_or_abs_path) and os.path.exists(rel_or_abs_path):
        return rel_or_abs_path

    # プロジェクトルートからの相対パス探索
    try:
        cur_file = os.path.abspath(__file__)
        editer_dir = os.path.dirname(os.path.dirname(cur_file))
        project_root = os.path.dirname(editer_dir)
        cand = os.path.join(project_root, rel_or_abs_path)
        if os.path.exists(cand):
            return cand
    except Exception:
        pass

    fixed_root = r"c:\Users\k024g\OneDrive\デスクトップ\study\3nenn\AL\Project\project"
    cand = os.path.join(fixed_root, rel_or_abs_path)
    if os.path.exists(cand):
        return cand

    return ""


def _get_or_create_terrain_material(mat_name, texture_path):
    """テクスチャ画像を貼った Principled BSDF マテリアルを作成・取得"""
    if mat_name in bpy.data.materials:
        return bpy.data.materials[mat_name]

    mat = bpy.data.materials.new(name=mat_name)
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    links = mat.node_tree.links

    bsdf = nodes.get("Principled BSDF")
    full_path = _resolve_texture_path(texture_path)
    if bsdf and full_path and os.path.exists(full_path):
        try:
            img = bpy.data.images.load(full_path, check_existing=True)
            tex_node = nodes.new(type="ShaderNodeTexImage")
            tex_node.image = img
            links.new(tex_node.outputs["Color"], bsdf.inputs["Base Color"])
        except Exception as e:
            print(f"[TerrainGen] テクスチャ読み込みエラー {full_path}: {e}")

    return mat


# ------------------------------------------
# Blender プレビューメッシュの生成
# ------------------------------------------

def _create_grid_mesh_data(
    mesh_name,
    size_x=100.0,
    size_y=100.0,
    div_x=20,
    div_y=20,
    texture_path="resources/Stagemap/863603.png",
    uv_tile=4.0,
):
    """
    面・UVタイリング・テクスチャマテリアル付きのグリッドメッシュデータ (bpy.types.Mesh) を生成。
    ローカル原点 (0, 0, 0) を中心とし、Z=0 の水平面。
    """
    mesh = bpy.data.meshes.new(mesh_name)

    verts = []
    faces = []
    div_x = max(int(div_x), 1)
    div_y = max(int(div_y), 1)
    step_x = size_x / div_x
    step_y = size_y / div_y
    start_x = -size_x * 0.5
    start_y = -size_y * 0.5

    # 頂点生成（ローカル座標系: Z=0 中心）
    for iy in range(div_y + 1):
        for ix in range(div_x + 1):
            vx = start_x + ix * step_x
            vy = start_y + iy * step_y
            verts.append((vx, vy, 0.0))

    # 面生成（反時計回り: Z+ 上向き法線）
    for iy in range(div_y):
        for ix in range(div_x):
            i0 = iy * (div_x + 1) + ix
            i1 = i0 + 1
            i2 = i0 + (div_x + 1)
            i3 = i2 + 1
            faces.append((i0, i1, i3, i2))

    mesh.from_pydata(verts, [], faces)

    # UV座標の設定（タイリング反映）
    uv_layer = mesh.uv_layers.new(name="UVMap")
    for poly in mesh.polygons:
        for loop_idx in poly.loop_indices:
            vert_idx = mesh.loops[loop_idx].vertex_index
            ix = vert_idx % (div_x + 1)
            iy = vert_idx // (div_x + 1)
            u = (ix / div_x) * uv_tile
            v = (iy / div_y) * uv_tile
            uv_layer.data[loop_idx].uv = (u, v)

    mesh.update()

    # テクスチャマテリアル適用
    if texture_path:
        mat_name = f"Mat_{mesh_name}"
        mat = _get_or_create_terrain_material(mat_name, texture_path)
        if mat:
            if not mesh.materials:
                mesh.materials.append(mat)
            else:
                mesh.materials[0] = mat

    return mesh


def _create_preview_grid_mesh(
    name,
    center_x=0.0,
    center_y=0.0,
    floor_z=0.0,
    size_x=100.0,
    size_y=100.0,
    div_x=20,
    div_y=20,
    texture_path="resources/Stagemap/863603.png",
    uv_tile=4.0,
    parent=None,
):
    """プレビュー用メッシュオブジェクトを生成（互換用）"""
    if name in bpy.data.objects:
        old = bpy.data.objects[name]
        bpy.data.objects.remove(old, do_unlink=True)

    mesh = _create_grid_mesh_data(
        name + "_mesh",
        size_x=size_x, size_y=size_y,
        div_x=div_x, div_y=div_y,
        texture_path=texture_path,
        uv_tile=uv_tile,
    )
    obj = bpy.data.objects.new(name, mesh)
    obj.show_wire = True
    bpy.context.collection.objects.link(obj)

    if parent:
        obj.parent = parent
        obj.location = (0.0, 0.0, 0.0)
    else:
        obj.location = (center_x, center_y, floor_z)

    return obj


# ------------------------------------------
# メイン生成関数
# ------------------------------------------

def generate_terrain_grid(
    margin=10.0,
    divisions_per_unit=2.0,
    max_divisions=64,
    floor_z_offset=-0.5,
    texture_path="resources/Stagemap/863603.png",
    uv_tile=4.0,
    terrain_name=_TERRAIN_OBJECT_NAME,
    create_preview=True,
):
    """
    StageRail の形状を読み取り、コースを包む terrain_grid TERRAIN オブジェクトを
    Blenderシーンに直接メッシュオブジェクト（MESH）として配置する。
    """
    rail = _get_stage_rail()
    if rail is None:
        return False, "StageRail が見つかりません。STAGE_RAIL タイプのカーブをシーンに配置してください。"

    sampled = _sample_curve_points(rail, num_samples=128)
    if not sampled:
        return False, "カーブのサンプリングに失敗しました。"

    cx, cy, raw_sx, raw_sy = _compute_rail_bounds(sampled)

    size_x = max(raw_sx + margin * 2.0, 10.0)
    size_y = max(raw_sy + margin * 2.0, 10.0)

    div_x = min(int(size_x * divisions_per_unit), max_divisions)
    div_y = min(int(size_y * divisions_per_unit), max_divisions)
    div_x = max(div_x, 4)
    div_y = max(div_y, 4)

    # 既存の同名オブジェクトを削除
    if terrain_name in bpy.data.objects:
        bpy.data.objects.remove(bpy.data.objects[terrain_name], do_unlink=True)
    preview_name = terrain_name + "_Preview"
    if preview_name in bpy.data.objects:
        bpy.data.objects.remove(bpy.data.objects[preview_name], do_unlink=True)

    # ─── TERRAIN メッシュオブジェクトを直接生成 ───
    # エディター上でも直接面・UV・テクスチャ付きの実体メッシュオブジェクトとして配置。
    t_mesh = _create_grid_mesh_data(
        f"Mesh_{terrain_name}",
        size_x=size_x, size_y=size_y,
        div_x=min(div_x, 32), div_y=min(div_y, 32),
        texture_path=texture_path,
        uv_tile=uv_tile,
    )
    obj = bpy.data.objects.new(terrain_name, t_mesh)
    obj.show_wire = True
    obj.location = (cx, cy, floor_z_offset)

    # ゲームに渡すカスタムプロパティを設定
    obj["object_type"]      = "TERRAIN"
    obj["file_name"]        = "terrain_grid"
    obj["texture"]          = texture_path
    obj["prop_size_x"]      = str(round(size_x, 2))
    obj["prop_size_y"]      = str(round(size_y, 2))
    obj["prop_divisions_x"] = str(div_x)
    obj["prop_divisions_y"] = str(div_y)
    obj["prop_uv_tile"]     = str(round(uv_tile, 2))

    bpy.context.collection.objects.link(obj)
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)

    msg = (
        f"地形生成完了！\n"
        f"  サイズ: {size_x:.1f} x {size_y:.1f} m\n"
        f"  分割数: {div_x} x {div_y}\n"
        f"  中心: ({cx:.1f}, {cy:.1f}), 高さ(Z): {floor_z_offset:.2f}\n"
        f"  テクスチャ: {texture_path}"
    )
    print(f"[TerrainGen] {msg}")
    return True, msg


# ==========================================
# Blender Operator
# ==========================================

class MYADDON_OT_generate_terrain(bpy.types.Operator):
    bl_idname = "myaddon.generate_terrain"
    bl_label = "コース地形を自動生成"
    bl_description = (
        "StageRail のコース形状を読み取り、"
        "ゲーム側の動的地形生成（terrain_grid）用 TERRAIN オブジェクトをシーンに配置します"
    )
    bl_options = {"REGISTER", "UNDO"}

    margin: bpy.props.FloatProperty(
        name="マージン（m）",
        description="コースのバウンディングボックスに付加するマージン",
        default=10.0,
        min=0.0,
        max=100.0,
    )  # type: ignore

    divisions_per_unit: bpy.props.FloatProperty(
        name="密度（分割数/m）",
        description="地形の 1m あたりの分割数（大きいほど滑らか）",
        default=1.0,
        min=0.1,
        max=5.0,
    )  # type: ignore

    max_divisions: bpy.props.IntProperty(
        name="最大分割数",
        description="1軸あたりの分割数の上限",
        default=64,
        min=4,
        max=256,
    )  # type: ignore

    floor_z_offset: bpy.props.FloatProperty(
        name="地形の高さ（Z）",
        description="地形を置く高さ（Blender Z軸 = ゲームの Y軸）",
        default=-0.5,
        min=-50.0,
        max=50.0,
    )  # type: ignore

    texture_path: bpy.props.StringProperty(
        name="テクスチャパス",
        description="地形に貼るテクスチャ（ゲームプロジェクト相対パス）",
        default="resources/Stagemap/863603.png",
    )  # type: ignore

    uv_tile: bpy.props.FloatProperty(
        name="UVタイル倍率",
        description="テクスチャのタイリング回数",
        default=4.0,
        min=0.1,
        max=50.0,
    )  # type: ignore

    create_preview: bpy.props.BoolProperty(
        name="プレビューメッシュを表示",
        description="Blenderビューポートに地形サイズのワイヤーフレームグリッドを表示します",
        default=True,
    )  # type: ignore

    def invoke(self, context, event):
        # StageRail があれば事前にサイズを見積もってパラメータに反映
        from terrain_generator import _get_stage_rail, _sample_curve_points, _compute_rail_bounds
        rail = _get_stage_rail()
        if rail:
            sampled = _sample_curve_points(rail, 128)
            if sampled:
                _, _, sx, sy = _compute_rail_bounds(sampled)
                # 予想サイズをラベルに使うだけで変更はしない
                self._estimated_size = (sx + self.margin * 2, sy + self.margin * 2)
            else:
                self._estimated_size = None
        else:
            self._estimated_size = None
        return context.window_manager.invoke_props_dialog(self, width=400)

    def draw(self, context):
        layout = self.layout
        layout.label(text="=== terrain_grid 自動地形生成 ===", icon="MESH_GRID")

        col = layout.column(align=True)
        col.prop(self, "margin")
        col.prop(self, "divisions_per_unit")
        col.prop(self, "max_divisions")
        col.prop(self, "floor_z_offset")

        layout.separator()
        layout.prop(self, "texture_path", icon="TEXTURE")
        layout.prop(self, "uv_tile")

        layout.separator()
        layout.prop(self, "create_preview", icon="HIDE_OFF")

        layout.separator()
        box = layout.box()
        if hasattr(self, "_estimated_size") and self._estimated_size:
            sx, sy = self._estimated_size
            div_x = min(int(sx * self.divisions_per_unit), self.max_divisions)
            div_y = min(int(sy * self.divisions_per_unit), self.max_divisions)
            box.label(text=f"推定サイズ: {sx:.0f} x {sy:.0f} m", icon="INFO")
            box.label(text=f"推定分割数: {div_x} x {div_y} = {div_x*div_y} 頂点グリッド", icon="MESH_DATA")
        else:
            box.label(text="StageRail が見つかりません", icon="ERROR")

        layout.separator()
        box2 = layout.box()
        box2.label(text="ゲームの動作:", icon="PLAY")
        box2.label(text="  terrain_grid 検出 → CreateTerrainModel() が動的にメッシュを生成")
        box2.label(text="  プレイヤーの足場・壁判定として機能します")

    def execute(self, context):
        ok, msg = generate_terrain_grid(
            margin=self.margin,
            divisions_per_unit=self.divisions_per_unit,
            max_divisions=self.max_divisions,
            floor_z_offset=self.floor_z_offset,
            texture_path=self.texture_path,
            uv_tile=self.uv_tile,
            create_preview=self.create_preview,
        )
        if not ok:
            self.report({"ERROR"}, msg)
            return {"CANCELLED"}

        first_line = msg.split("\n")[0]
        self.report({"INFO"}, first_line)
        return {"FINISHED"}


# ==========================================
# メッシュ (OBJModel) → terrain_grid 変換
# ==========================================

def _compute_mesh_bounds(obj):
    """
    メッシュオブジェクトのワールド座標系におけるバウンディングボックスを計算。
    returns: (cx, cy, cz, sx, sy, sz, min_z, max_z) or None
    """
    if not obj or obj.type != 'MESH':
        return None
    try:
        mat = obj.matrix_world
        world_bbox = [mat @ mathutils.Vector(corner) for corner in obj.bound_box]
        xs = [v.x for v in world_bbox]
        ys = [v.y for v in world_bbox]
        zs = [v.z for v in world_bbox]
        min_x, max_x = min(xs), max(xs)
        min_y, max_y = min(ys), max(ys)
        min_z, max_z = min(zs), max(zs)
        cx = (min_x + max_x) * 0.5
        cy = (min_y + max_y) * 0.5
        cz = (min_z + max_z) * 0.5
        sx = max(max_x - min_x, 0.1)
        sy = max(max_y - min_y, 0.1)
        sz = max(max_z - min_z, 0.0)
        return (cx, cy, cz, sx, sy, sz, min_z, max_z)
    except Exception as e:
        print(f"[TerrainGen] バウンディングボックス計算エラー: {e}")
        return None


def _detect_mesh_texture(obj):
    """メッシュオブジェクトからテクスチャパスを推定"""
    if "texture" in obj and obj["texture"]:
        return str(obj["texture"])
    if obj.data and hasattr(obj.data, "materials"):
        for mat in obj.data.materials:
            if mat and mat.use_nodes and mat.node_tree:
                for node in mat.node_tree.nodes:
                    if node.type == 'TEX_IMAGE' and node.image and node.image.filepath:
                        raw = node.image.filepath.replace("\\", "/")
                        if raw.startswith("//"):
                            raw = raw[2:]
                        idx = raw.find("resources/")
                        if idx != -1:
                            return raw[idx:]
                        import os
                        return os.path.basename(raw)
    return "resources/Stagemap/863603.png"


def convert_mesh_to_terrain_grid(
    target_obj,
    margin=0.0,
    divisions_per_unit=1.0,
    max_divisions=64,
    floor_z_mode="ORIGIN",
    custom_floor_z=0.0,
    texture_path="",
    uv_tile=4.0,
    replace_mode="REPLACE",
    create_preview=True,
):
    """
    既存のメッシュオブジェクト（OBJModel等）から terrain_grid TERRAIN オブジェクトへ変換する。

    Parameters
    ----------
    target_obj        : 変換元のメッシュオブジェクト
    margin            : スパンに付加する余白（m）
    divisions_per_unit: 1mあたりの分割密度
    max_divisions     : 分割数の上限
    floor_z_mode      : "ORIGIN" (原点Z), "TOP" (上面), "BOTTOM" (下面), "CUSTOM" (指定値)
    custom_floor_z    : floor_z_mode == "CUSTOM" のときの高さ
    texture_path      : テクスチャパス（空なら自動検出）
    uv_tile           : UVタイリング倍率
    replace_mode      : "REPLACE" (元モデル削除), "HIDE" (元モデル非表示化), "KEEP" (残す)
    create_preview    : プレビュー用ワイヤーフレームを生成するか

    Returns
    -------
    (bool, str) = (成功フラグ, メッセージ)
    """
    if not target_obj or target_obj.type != 'MESH':
        return False, "変換対象のメッシュオブジェクトが選択されていません。"

    bounds = _compute_mesh_bounds(target_obj)
    if not bounds:
        return False, f"オブジェクト '{target_obj.name}' のバウンディングボックス計算に失敗しました。"

    cx, cy, cz, raw_sx, raw_sy, raw_sz, min_z, max_z = bounds

    # マージン付加
    size_x = max(raw_sx + margin * 2.0, 1.0)
    size_y = max(raw_sy + margin * 2.0, 1.0)

    # 分割数
    div_x = min(int(size_x * divisions_per_unit), max_divisions)
    div_y = min(int(size_y * divisions_per_unit), max_divisions)
    div_x = max(div_x, 4)
    div_y = max(div_y, 4)

    # 高さの決定
    if floor_z_mode == "ORIGIN":
        floor_z = target_obj.location.z
    elif floor_z_mode == "TOP":
        floor_z = max_z
    elif floor_z_mode == "BOTTOM":
        floor_z = min_z
    else:  # CUSTOM
        floor_z = custom_floor_z

    # テクスチャパス決定
    tex = texture_path.strip() if texture_path else _detect_mesh_texture(target_obj)

    # オブジェクト名決定
    original_name = target_obj.name
    if replace_mode == "REPLACE":
        terrain_name = original_name
        # 先に元オブジェクトを削除
        bpy.data.objects.remove(target_obj, do_unlink=True)
    elif replace_mode == "HIDE":
        terrain_name = original_name + "_TerrainGrid"
        target_obj.hide_viewport = True
        target_obj.hide_render = True
        target_obj["disabled"] = True
    else:  # KEEP
        terrain_name = original_name + "_TerrainGrid"

    # 既存の同名オブジェクトやプレビューがあれば削除
    if terrain_name in bpy.data.objects:
        bpy.data.objects.remove(bpy.data.objects[terrain_name], do_unlink=True)
    preview_name = terrain_name + "_Preview"
    if preview_name in bpy.data.objects:
        bpy.data.objects.remove(bpy.data.objects[preview_name], do_unlink=True)

    # ─── TERRAIN メッシュオブジェクトを直接生成 ───
    t_mesh = _create_grid_mesh_data(
        f"Mesh_{terrain_name}",
        size_x=size_x, size_y=size_y,
        div_x=min(div_x, 32), div_y=min(div_y, 32),
        texture_path=tex,
        uv_tile=uv_tile,
    )
    obj = bpy.data.objects.new(terrain_name, t_mesh)
    obj.show_wire = True
    obj.location = (cx, cy, floor_z)

    # カスタムプロパティ設定
    obj["object_type"]      = "TERRAIN"
    obj["file_name"]        = "terrain_grid"
    obj["texture"]          = tex
    obj["prop_size_x"]      = str(round(size_x, 2))
    obj["prop_size_y"]      = str(round(size_y, 2))
    obj["prop_divisions_x"] = str(div_x)
    obj["prop_divisions_y"] = str(div_y)
    obj["prop_uv_tile"]     = str(round(uv_tile, 2))

    bpy.context.collection.objects.link(obj)
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)

    msg = (
        f"OBJモデル '{original_name}' を地形グリッドに変換しました！\n"
        f"  地形オブジェクト名: {terrain_name}\n"
        f"  サイズ: {size_x:.1f} x {size_y:.1f} m (元: {raw_sx:.1f} x {raw_sy:.1f} m)\n"
        f"  分割数: {div_x} x {div_y}\n"
        f"  中心: ({cx:.1f}, {cy:.1f}), 高さ(Z): {floor_z:.2f}\n"
        f"  テクスチャ: {tex}"
    )
    print(f"[TerrainGen] {msg}")
    return True, msg


class MYADDON_OT_convert_obj_to_terrain(bpy.types.Operator):
    bl_idname = "myaddon.convert_obj_to_terrain"
    bl_label = "OBJモデルを地形グリッドに変換"
    bl_description = (
        "選択中のメッシュオブジェクト（OBJModel等）のサイズ・位置・テクスチャから、"
        "ゲーム側の動的地形生成（terrain_grid）用 TERRAIN オブジェクトへ変換します"
    )
    bl_options = {"REGISTER", "UNDO"}

    margin: bpy.props.FloatProperty(
        name="余白マージン（m）",
        description="元のメッシュ境界に付加する余白",
        default=0.0,
        min=0.0,
        max=100.0,
    )  # type: ignore

    divisions_per_unit: bpy.props.FloatProperty(
        name="密度（分割数/m）",
        description="地形の 1m あたりの分割数",
        default=1.0,
        min=0.1,
        max=5.0,
    )  # type: ignore

    max_divisions: bpy.props.IntProperty(
        name="最大分割数",
        description="1軸あたりの分割数の上限",
        default=64,
        min=4,
        max=256,
    )  # type: ignore

    floor_z_mode: bpy.props.EnumProperty(
        name="高さ基準（Z）",
        description="地形のZ座標（高さ）をどこに合わせるか",
        items=[
            ("ORIGIN", "原点 (Origin Z)", "メッシュオブジェクトの原点Z位置に合わせる"),
            ("TOP", "上面 (Top Z)", "メッシュの最高点Zに合わせる"),
            ("BOTTOM", "下面 (Bottom Z)", "メッシュの最底点Zに合わせる"),
            ("CUSTOM", "カスタム指定", "下の「カスタム高さ」で指定した値にする"),
        ],
        default="ORIGIN",
    )  # type: ignore

    custom_floor_z: bpy.props.FloatProperty(
        name="カスタム高さ（Z）",
        description="floor_z_mode が CUSTOM の場合の地形高さ",
        default=0.0,
        min=-100.0,
        max=100.0,
    )  # type: ignore

    texture_path: bpy.props.StringProperty(
        name="テクスチャパス",
        description="地形テクスチャの相対パス（空欄なら元モデルから自動取得）",
        default="",
    )  # type: ignore

    uv_tile: bpy.props.FloatProperty(
        name="UVタイル倍率",
        description="テクスチャのタイリング倍率",
        default=4.0,
        min=0.1,
        max=50.0,
    )  # type: ignore

    replace_mode: bpy.props.EnumProperty(
        name="元モデルの処理",
        description="変換後に元のOBJメッシュをどう処理するか",
        items=[
            ("REPLACE", "置換（元モデルを削除）", "元メッシュを削除し、同じ名前で地形グリッドを配置します（推奨）"),
            ("HIDE", "非表示化（バックアップ保持）", "元メッシュを非表示・disabled=Trueにし、別名で地形グリッドを配置します"),
            ("KEEP", "そのまま保持", "元メッシュをそのまま残し、別名で地形グリッドを配置します"),
        ],
        default="REPLACE",
    )  # type: ignore

    create_preview: bpy.props.BoolProperty(
        name="プレビューメッシュを表示",
        description="Blenderビューポートにワイヤーフレームのグリッドを表示します",
        default=True,
    )  # type: ignore

    @classmethod
    def poll(cls, context):
        return context.active_object is not None and context.active_object.type == 'MESH'

    def invoke(self, context, event):
        obj = context.active_object
        if not obj or obj.type != 'MESH':
            self.report({"ERROR"}, "メッシュオブジェクトを選択してください。")
            return {"CANCELLED"}

        bounds = _compute_mesh_bounds(obj)
        if bounds:
            cx, cy, cz, sx, sy, sz, min_z, max_z = bounds
            self._mesh_info = {
                "name": obj.name,
                "size_x": sx,
                "size_y": sy,
                "size_z": sz,
                "center": (cx, cy, cz),
                "min_z": min_z,
                "max_z": max_z,
                "origin_z": obj.location.z,
            }
        else:
            self._mesh_info = None

        # テクスチャを事前スキャン
        if not self.texture_path:
            self.texture_path = _detect_mesh_texture(obj)

        return context.window_manager.invoke_props_dialog(self, width=420)

    def draw(self, context):
        layout = self.layout
        layout.label(text="=== OBJModel → 地形グリッド変換 ===", icon="MOD_OCEAN")

        if hasattr(self, "_mesh_info") and self._mesh_info:
            info = self._mesh_info
            box_info = layout.box()
            box_info.label(text=f"元モデル: {info['name']}", icon="MESH_DATA")
            box_info.label(text=f"サイズ: 幅(X) {info['size_x']:.2f}m × 奥(Y) {info['size_y']:.2f}m × 高(Z) {info['size_z']:.2f}m")
            box_info.label(text=f"Z範囲: {info['min_z']:.2f}m ～ {info['max_z']:.2f}m (原点: {info['origin_z']:.2f}m)")

        col = layout.column(align=True)
        col.prop(self, "margin")
        col.prop(self, "divisions_per_unit")
        col.prop(self, "max_divisions")

        layout.separator()
        col_z = layout.column(align=True)
        col_z.prop(self, "floor_z_mode")
        if self.floor_z_mode == "CUSTOM":
            col_z.prop(self, "custom_floor_z")

        layout.separator()
        layout.prop(self, "texture_path", icon="TEXTURE")
        layout.prop(self, "uv_tile")

        layout.separator()
        layout.prop(self, "replace_mode", icon="OUTLINER_OB_MESH")
        layout.prop(self, "create_preview", icon="HIDE_OFF")

        # 変換後推定
        if hasattr(self, "_mesh_info") and self._mesh_info:
            info = self._mesh_info
            ex_sx = info['size_x'] + self.margin * 2.0
            ex_sy = info['size_y'] + self.margin * 2.0
            ex_dx = max(min(int(ex_sx * self.divisions_per_unit), self.max_divisions), 4)
            ex_dy = max(min(int(ex_sy * self.divisions_per_unit), self.max_divisions), 4)

            layout.separator()
            box_calc = layout.box()
            box_calc.label(text=f"変換後サイズ: {ex_sx:.1f} x {ex_sy:.1f} m", icon="INFO")
            box_calc.label(text=f"変換後分割数: {ex_dx} x {ex_dy} ({ex_dx*ex_dy} 頂点グリッド)", icon="GRID")

    def execute(self, context):
        obj = context.active_object
        if not obj or obj.type != 'MESH':
            self.report({"ERROR"}, "メッシュオブジェクトが選択されていません。")
            return {"CANCELLED"}

        ok, msg = convert_mesh_to_terrain_grid(
            target_obj=obj,
            margin=self.margin,
            divisions_per_unit=self.divisions_per_unit,
            max_divisions=self.max_divisions,
            floor_z_mode=self.floor_z_mode,
            custom_floor_z=self.custom_floor_z,
            texture_path=self.texture_path,
            uv_tile=self.uv_tile,
            replace_mode=self.replace_mode,
            create_preview=self.create_preview,
        )
        if not ok:
            self.report({"ERROR"}, msg)
            return {"CANCELLED"}

        first_line = msg.split("\n")[0]
        self.report({"INFO"}, first_line)
        return {"FINISHED"}


# ------------------------------------------
# 登録・解除（単体実行用）
# ------------------------------------------
classes = (
    MYADDON_OT_generate_terrain,
    MYADDON_OT_convert_obj_to_terrain,
)

def register():
    for cls in classes:
        try:
            bpy.utils.register_class(cls)
        except ValueError:
            pass

def unregister():
    for cls in reversed(classes):
        try:
            bpy.utils.unregister_class(cls)
        except Exception:
            pass

if __name__ == "__main__":
    register()


