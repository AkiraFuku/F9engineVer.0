import bpy
import bpy_extras
import json
import math
import mathutils
import os


# ==========================================
# レールベジェ曲線計算 & スナップユーティリティ
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

    if "StageRail" in bpy.data.objects and bpy.data.objects["StageRail"].type == 'CURVE':
        return bpy.data.objects["StageRail"]

    for obj in bpy.data.objects:
        if obj.type == 'CURVE' and obj.get("object_type") == "STAGE_RAIL":
            return obj

    for obj in bpy.data.objects:
        if obj.type == 'CURVE':
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

    print(f"  [RailSnap] {obj.name}: rail_pos=[{t}, {height_offset}] -> Location: ({final_world_pos.x:.2f}, {final_world_pos.y:.2f}, {final_world_pos.z:.2f})")
    return True


def snap_all_objects_to_rail():
    """シーン内の rail_pos を持つ全オブジェクトをレール上に配置"""
    count = 0
    for obj in bpy.data.objects:
        if "rail_pos" in obj:
            if apply_rail_position_to_object(obj):
                count += 1
    return count


# ==========================================
# プレビューモデル（メッシュ）読み込み & キャッシュ
# ==========================================
_preview_mesh_cache = {}

def get_project_root():
    """プロジェクトのルートディレクトリを取得"""
    try:
        cur_file = os.path.abspath(__file__)
        level_editer_dir = os.path.dirname(cur_file)
        editer_dir = os.path.dirname(level_editer_dir)
        cand = os.path.dirname(editer_dir)
        if os.path.exists(os.path.join(cand, "resources")):
            return cand
    except Exception:
        pass

    try:
        blend_path = bpy.data.filepath
        if blend_path:
            cand = os.path.dirname(blend_path)
            for _ in range(5):
                if os.path.exists(os.path.join(cand, "resources")):
                    return cand
                cand = os.path.dirname(cand)
    except Exception:
        pass

    fixed_path = r"c:\Users\k024g\OneDrive\デスクトップ\study\3nenn\AL\Project\project"
    if os.path.exists(fixed_path):
        return fixed_path

    return ""


def create_texture_material(mat_name, texture_path):
    """テクスチャ画像を貼った Principled BSDF マテリアルを作成・取得"""
    if mat_name in bpy.data.materials:
        return bpy.data.materials[mat_name]

    mat = bpy.data.materials.new(name=mat_name)
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    links = mat.node_tree.links

    bsdf = nodes.get("Principled BSDF")
    if bsdf and os.path.exists(texture_path):
        try:
            img = bpy.data.images.load(texture_path, check_existing=True)
            tex_node = nodes.new(type="ShaderNodeTexImage")
            tex_node.image = img
            links.new(tex_node.outputs["Color"], bsdf.inputs["Base Color"])
        except Exception as e:
            print(f"Error loading texture {texture_path}: {e}")

    return mat


def load_obj_mesh_converted(filepath, tex_path=None, mesh_name=None):
    """
    ゲーム座標系(Y-up: X右, Y上, Z奥)で作られたOBJファイルを読み込み、
    Blender座標系(Z-up: X右, Y奥, Z上)へ頂点・法線を変換したメッシュを生成
    """
    if not os.path.exists(filepath):
        return None

    prev_selected = [o for o in bpy.context.selected_objects]
    prev_active = bpy.context.view_layer.objects.active

    imported_objects = []
    try:
        if hasattr(bpy.ops.wm, "obj_import"):
            try:
                bpy.ops.wm.obj_import(filepath=filepath, forward_axis='Y', up_axis='Z')
            except Exception:
                bpy.ops.wm.obj_import(filepath=filepath)
        elif hasattr(bpy.ops.import_scene, "obj"):
            try:
                bpy.ops.import_scene.obj(filepath=filepath, axis_forward='Y', axis_up='Z')
            except Exception:
                bpy.ops.import_scene.obj(filepath=filepath)
        imported_objects = [o for o in bpy.context.selected_objects if o not in prev_selected]
    except Exception as e:
        print(f"Error importing OBJ {filepath}: {e}")
        return None

    if not imported_objects:
        return None

    mesh_obj = None
    for io in imported_objects:
        if io.type == 'MESH':
            mesh_obj = io
            break

    if not mesh_obj:
        for io in imported_objects:
            bpy.data.objects.remove(io, do_unlink=True)
        return None

    mesh_data = mesh_obj.data.copy()
    if mesh_name:
        mesh_data.name = mesh_name

    # ゲーム座標系 (X右, Y上, Z奥) -> Blender座標系 (X右, Y奥, Z上) への座標軸変換
    mat_game_to_blender = mathutils.Matrix((
        (1.0, 0.0, 0.0, 0.0),
        (0.0, 0.0, 1.0, 0.0),
        (0.0, 1.0, 0.0, 0.0),
        (0.0, 0.0, 0.0, 1.0)
    ))
    mesh_data.transform(mat_game_to_blender)
    mesh_data.flip_normals()
    mesh_data.update()

    if tex_path and os.path.exists(tex_path):
        mat = create_texture_material(f"Mat_{mesh_data.name}", tex_path)
        if not mesh_data.materials:
            mesh_data.materials.append(mat)
        else:
            mesh_data.materials[0] = mat

    for io in imported_objects:
        bpy.data.objects.remove(io, do_unlink=True)

    for o in prev_selected:
        try:
            o.select_set(True)
        except Exception:
            pass
    if prev_active and prev_active.name in bpy.data.objects:
        bpy.context.view_layer.objects.active = prev_active

    return mesh_data


def get_or_load_preview_mesh(obj_type):
    """ENEMY, PLAYER_SPAWN, GOAL 用のプレビューメッシュを取得（ゲーム座標 -> Blender座標変換済み・キャッシュ付き）"""
    global _preview_mesh_cache
    if obj_type in _preview_mesh_cache:
        mesh = _preview_mesh_cache[obj_type]
        if mesh and mesh.name in bpy.data.meshes:
            return mesh

    mesh_name = f"Mesh_Preview_{obj_type}"
    if mesh_name in bpy.data.meshes:
        _preview_mesh_cache[obj_type] = bpy.data.meshes[mesh_name]
        return bpy.data.meshes[mesh_name]

    root = get_project_root()
    model_configs = {
        "ENEMY": {
            "obj_rel": os.path.join("resources", "taru", "taru.obj"),
            "tex_rel": os.path.join("resources", "taru", "taru.png"),
        },
        "PLAYER_SPAWN": {
            "obj_rel": os.path.join("resources", "player", "player.obj"),
            "tex_rel": os.path.join("resources", "player", "player.png"),
        },
        "GOAL": {
            "obj_rel": os.path.join("resources", "Goal", "goal.obj"),
            "tex_rel": os.path.join("resources", "Goal", "goal.png"),
        },
    }

    cfg = model_configs.get(obj_type)
    if not cfg:
        return None

    obj_path = os.path.join(root, cfg["obj_rel"]) if root else ""
    tex_path = os.path.join(root, cfg["tex_rel"]) if root else ""

    if os.path.exists(obj_path):
        mesh_data = load_obj_mesh_converted(obj_path, tex_path, mesh_name=mesh_name)
        if mesh_data:
            _preview_mesh_cache[obj_type] = mesh_data
            return mesh_data

    # フォールバック
    mesh_data = bpy.data.meshes.new(mesh_name)
    verts = [(-0.5, -0.5, -0.5), (0.5, -0.5, -0.5), (0.5, 0.5, -0.5), (-0.5, 0.5, -0.5),
             (-0.5, -0.5,  0.5), (0.5, -0.5,  0.5), (0.5, 0.5,  0.5), (-0.5, 0.5,  0.5)]
    faces = [(0, 1, 2, 3), (4, 7, 6, 5), (0, 4, 5, 1), (1, 5, 6, 2), (2, 6, 7, 3), (4, 0, 3, 7)]
    mesh_data.from_pydata(verts, [], faces)
    mesh_data.update()
    _preview_mesh_cache[obj_type] = mesh_data
    return mesh_data


# ==========================================
# JSON ステージデータ インポーター
# ==========================================

def _import_object_recursive(objects_json, parent=None, clear_existing=True, convert_coords=True):
    """JSON オブジェクトリストを再帰的に Blender オブジェクトへ変換"""
    for obj_data in objects_json:
        name        = obj_data.get("name", "Unnamed")
        type_str    = obj_data.get("type", "MESH")
        obj_type    = obj_data.get("object_type", "")
        transform   = obj_data.get("transform", {})
        file_name   = obj_data.get("file_name", "")
        model_dir   = obj_data.get("model_dir", "")
        texture     = obj_data.get("texture", "")
        disabled    = obj_data.get("disabled", False)
        rail_pos    = obj_data.get("rail_pos", None)
        enemy_type  = obj_data.get("enemy_type", "")
        loop        = obj_data.get("loop", False)
        rail_points = obj_data.get("rail_points", [])
        collider    = obj_data.get("collider", {})
        event_name  = obj_data.get("event_name", "")
        fire_mode   = obj_data.get("fire_mode", "")
        properties  = obj_data.get("properties", {})
        children    = obj_data.get("children", [])

        tr_raw = transform.get("translation", [0, 0, 0])
        sc_raw = transform.get("scaling",     [1, 1, 1])
        ro_raw = transform.get("rotation",    [0, 0, 0])

        if convert_coords:
            # ゲーム (X右, Y上, Z奥) -> Blender (X右, Y奥, Z上)
            loc   = (tr_raw[0], tr_raw[2], tr_raw[1])
            scale = (sc_raw[0], sc_raw[2], sc_raw[1])
            rot_rad = (
                -math.radians(ro_raw[0]),
                 math.radians(ro_raw[2]),
                 math.radians(ro_raw[1]),
            )
        else:
            loc   = (tr_raw[0], tr_raw[1], tr_raw[2])
            scale = (sc_raw[0], sc_raw[1], sc_raw[2])
            rot_rad = (
                math.radians(ro_raw[0]),
                math.radians(ro_raw[1]),
                math.radians(ro_raw[2]),
            )

        # 既存オブジェクトの削除
        if clear_existing and name in bpy.data.objects:
            bpy.data.objects.remove(bpy.data.objects[name], do_unlink=True)

        blender_obj = None

        # CURVE (レール)
        if type_str == "CURVE" and rail_points:
            curve_data = bpy.data.curves.new(name=name, type="CURVE")
            curve_data.dimensions = "3D"
            curve_data.resolution_u = 12
            spline = curve_data.splines.new("BEZIER")
            spline.use_cyclic_u = loop
            spline.bezier_points.add(len(rail_points) - 1)
            for i, rp in enumerate(rail_points):
                co = rp.get("co", [0, 0, 0])
                hl = rp.get("handle_left",  co)
                hr = rp.get("handle_right", co)
                bpt = spline.bezier_points[i]
                if convert_coords:
                    # ゲーム (X右, Y上, Z奥) -> Blender (X右, Y奥, Z上)
                    bpt.co           = (co[0], co[2], co[1])
                    bpt.handle_left  = (hl[0], hl[2], hl[1])
                    bpt.handle_right = (hr[0], hr[2], hr[1])
                else:
                    bpt.co           = (co[0], co[1], co[2])
                    bpt.handle_left  = (hl[0], hl[1], hl[2])
                    bpt.handle_right = (hr[0], hr[1], hr[2])
                bpt.handle_left_type  = "FREE"
                bpt.handle_right_type = "FREE"
            blender_obj = bpy.data.objects.new(name, curve_data)
            bpy.context.collection.objects.link(blender_obj)

        elif type_str == "EMPTY":
            # ENEMY, PLAYER_SPAWN, GOAL は 3D座標(Empty)ではなくモデルメッシュで描画
            preview_mesh = None
            if obj_type in ["ENEMY", "PLAYER_SPAWN", "GOAL"]:
                preview_mesh = get_or_load_preview_mesh(obj_type)

            if preview_mesh:
                blender_obj = bpy.data.objects.new(name, preview_mesh)
            else:
                blender_obj = bpy.data.objects.new(name, None)
                blender_obj.empty_display_type = 'PLAIN_AXES'
            bpy.context.collection.objects.link(blender_obj)

        else:
            # MESH オブジェクト (file_name がある場合はモデルを読み込んでゲーム座標変換)
            loaded_mesh = None
            if file_name and convert_coords:
                root = get_project_root()
                possible_paths = []
                if model_dir:
                    possible_paths.append(os.path.join(root, model_dir, file_name))
                    possible_paths.append(os.path.join(root, model_dir, file_name + ".obj"))
                possible_paths.append(os.path.join(root, "resources", "Stagemap", file_name))
                possible_paths.append(os.path.join(root, "resources", file_name))
                possible_paths.append(os.path.join(root, file_name))

                for p in possible_paths:
                    if os.path.exists(p) and p.lower().endswith(".obj"):
                        tex_full = os.path.join(root, texture) if (texture and root) else ""
                        loaded_mesh = load_obj_mesh_converted(p, tex_full, mesh_name=f"Mesh_{name}")
                        if loaded_mesh:
                            break

            if loaded_mesh:
                blender_obj = bpy.data.objects.new(name, loaded_mesh)
            else:
                mesh_data = bpy.data.meshes.new(name)
                verts = [
                    (-1, -1, -1), (1, -1, -1), (1, 1, -1), (-1, 1, -1),
                    (-1, -1,  1), (1, -1,  1), (1, 1,  1), (-1, 1,  1)
                ]
                faces = [
                    (0, 1, 2, 3), (4, 7, 6, 5), (0, 4, 5, 1),
                    (1, 5, 6, 2), (2, 6, 7, 3), (4, 0, 3, 7)
                ]
                mesh_data.from_pydata(verts, [], faces)
                mesh_data.update()
                blender_obj = bpy.data.objects.new(name, mesh_data)
            bpy.context.collection.objects.link(blender_obj)

        if blender_obj:
            blender_obj.location       = loc
            blender_obj.rotation_euler = rot_rad
            blender_obj.scale          = scale

            # カスタムプロパティ復元
            if obj_type:   blender_obj["object_type"] = obj_type
            if file_name:  blender_obj["file_name"]   = file_name
            if model_dir:  blender_obj["model_dir"]   = model_dir
            if texture:    blender_obj["texture"]     = texture
            if disabled:   blender_obj["disabled"]    = disabled
            if enemy_type: blender_obj["enemy_type"]  = enemy_type
            if loop:       blender_obj["loop"]        = loop
            if event_name: blender_obj["event_name"]  = event_name
            if fire_mode:  blender_obj["fire_mode"]   = fire_mode
            if rail_pos is not None: blender_obj["rail_pos"] = rail_pos

            # コライダー復元
            if collider:
                blender_obj["collider"] = collider.get("type", "BOX")
                cc = collider.get("center", [0, 0, 0])
                cs = collider.get("size", [1, 1, 1])
                if convert_coords:
                    blender_obj["collider_center"] = [cc[0], cc[2], cc[1]]
                    blender_obj["collider_size"]   = [cs[0], cs[2], cs[1]]
                else:
                    blender_obj["collider_center"] = cc
                    blender_obj["collider_size"]   = cs

            for k, v in properties.items():
                blender_obj[k] = v

            if parent:
                blender_obj.parent = parent

            if children:
                _import_object_recursive(children, parent=blender_obj,
                                         clear_existing=clear_existing,
                                         convert_coords=convert_coords)


class MYADDON_OT_import_scene(bpy.types.Operator, bpy_extras.io_utils.ImportHelper):
    bl_idname    = "myaddon.import_scene"
    bl_label     = "シーンロード"
    bl_description = "ステージ JSON ファイルを読み込んでシーンに配置します"

    filename_ext = ".json"
    filter_glob: bpy.props.StringProperty(default="*.json", options={"HIDDEN"})

    clear_scene: bpy.props.BoolProperty(
        name="既存オブジェクトを上書き",
        description="同名オブジェクトが存在する場合に削除して再作成します",
        default=True,
    )

    convert_from_game_coords: bpy.props.BoolProperty(
        name="ゲーム座標系から変換 (Game -> Blender)",
        description="ゲーム座標系(X右, Y上, Z奥)からBlender座標系(X右, Y奥, Z上)に変換して配置します",
        default=True,
    )

    snap_to_rails: bpy.props.BoolProperty(
        name="rail_pos をレール上に自動配置",
        description="rail_pos を持つオブジェクト（Player, Enemy, Goal等）をベジェレール上の位置へ自動配置します",
        default=True,
    )

    def execute(self, context):
        try:
            with open(self.filepath, "rt", encoding="utf-8") as f:
                data = json.load(f)
        except Exception as e:
            self.report({"ERROR"}, f"JSON 読み込み失敗: {e}")
            return {"CANCELLED"}

        objects = data.get("objects", [])
        if not objects:
            self.report({"WARNING"}, "オブジェクトが見つかりませんでした")
            return {"FINISHED"}

        should_convert = self.convert_from_game_coords
        _import_object_recursive(objects, parent=None, clear_existing=self.clear_scene, convert_coords=should_convert)

        # rail_pos を持つオブジェクトをベジェレール上に自動スナップ
        snapped_count = 0
        if self.snap_to_rails:
            snapped_count = snap_all_objects_to_rail()

        msg = f"{len(objects)} 個のオブジェクトを配置しました"
        if snapped_count > 0:
            msg += f" (レール自動配置: {snapped_count}個)"
        msg += f" (ゲーム座標変換: {'ON' if should_convert else 'OFF'})"
        self.report({"INFO"}, msg)
        return {"FINISHED"}

    def draw(self, context):
        self.layout.prop(self, "clear_scene")
        self.layout.prop(self, "convert_from_game_coords")
        self.layout.prop(self, "snap_to_rails")
