import bpy
import math
import bpy_extras
import json
import gpu
import gpu_extras.batch
import copy
import mathutils
import os

bl_info = {
    "name": "レベルエディタ",
    "author": "Fuku Akira",
    "version": (1, 2),
    "blender": (3, 3, 1),
    "location": "TopBar > MyMenu / Object Properties",
    "description": "ステージエディタ（シーン出力・シーンロード・座標系変換対応）",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Object"
}

# ==========================================
# 1. 頂点を伸ばす
# ==========================================
class MYADDON_OT_stretch_vertex(bpy.types.Operator):
    bl_idname = "myaddon.myaddon_ot_stretch_vertex"
    bl_label = "頂点を伸ばす"
    bl_description = "頂点座標を引っ張って伸ばします"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        if "Cube" in bpy.data.objects and bpy.data.objects["Cube"].data.vertices:
            bpy.data.objects["Cube"].data.vertices[0].co.x += 1.0
        return {'FINISHED'}


# ==========================================
# 2. オブジェクトタイプ設定用
# ==========================================
class OBJECT_PT_type_settings(bpy.types.Panel):
    """オブジェクトの基本タイプを設定するパネル"""
    bl_idname = "OBJECT_PT_type_settings"
    bl_label = "Object Type Settings"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "object"

    def draw(self, context):
        layout = self.layout
        obj = context.object

        if "object_type" not in obj:
            layout.operator("myaddon.init_object_type", text="オブジェクトタイプを初期化")
        else:
            row = layout.row(align=True)
            row.label(text=f"現在のタイプ: {obj['object_type']}")
            
            layout.separator()
            row = layout.row(align=True)
            row.operator("myaddon.set_object_type", text="ENEMYにする").target_type = "ENEMY"
            row.operator("myaddon.set_object_type", text="PLAYERにする").target_type = "PLAYER_SPAWN"
            row.operator("myaddon.set_object_type", text="GOALにする").target_type = "GOAL"
            row.operator("myaddon.set_object_type", text="PROP(背景)にする").target_type = "PROP"

        layout.separator()
        layout.operator("myaddon.apply_all_preview_models", text="全Enemy/Playerをモデル表示に更新", icon='MESH_DATA')


class MYADDON_OT_init_object_type(bpy.types.Operator):
    bl_idname = "myaddon.init_object_type"
    bl_label = "オブジェクトタイプ初期化"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        context.object["object_type"] = "PROP"
        return {"FINISHED"}


class MYADDON_OT_set_object_type(bpy.types.Operator):
    bl_idname = "myaddon.set_object_type"
    bl_label = "タイプ変更"
    bl_options = {"REGISTER", "UNDO"}

    target_type: bpy.props.StringProperty()  # type: ignore

    def execute(self, context):
        obj = context.object
        obj["object_type"] = self.target_type

        # ENEMY, PLAYER_SPAWN, GOAL に設定された場合、プレビューモデルメッシュを自動適用
        if self.target_type in ["ENEMY", "PLAYER_SPAWN", "GOAL"]:
            mesh = get_or_load_preview_mesh(self.target_type)
            if mesh:
                if obj.type == 'MESH':
                    obj.data = mesh
                elif obj.type == 'EMPTY':
                    new_obj = bpy.data.objects.new(obj.name + "_mesh_tmp", mesh)
                    new_obj.matrix_world = obj.matrix_world.copy()
                    for k, v in obj.items():
                        new_obj[k] = v
                    col = obj.users_collection[0] if obj.users_collection else context.collection
                    col.objects.link(new_obj)
                    old_name = obj.name
                    bpy.data.objects.remove(obj, do_unlink=True)
                    new_obj.name = old_name
                    context.view_layer.objects.active = new_obj
                    new_obj.select_set(True)

        return {"FINISHED"}


class MYADDON_OT_apply_all_preview_models(bpy.types.Operator):
    bl_idname = "myaddon.apply_all_preview_models"
    bl_label = "全Enemy/Playerをモデル表示に更新"
    bl_description = "シーン内の ENEMY, PLAYER_SPAWN, GOAL をモデルメッシュ表示に更新・変換します"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        global _preview_mesh_cache
        _preview_mesh_cache.clear()
        # 既存のプレビューメッシュを削除して再読み込みを保証
        for ot in ["ENEMY", "PLAYER_SPAWN", "GOAL"]:
            m_name = f"Mesh_Preview_{ot}"
            if m_name in bpy.data.meshes:
                bpy.data.meshes.remove(bpy.data.meshes[m_name], do_unlink=True)

        count = 0
        for obj in list(bpy.data.objects):
            ot = obj.get("object_type", "")
            if ot in ["ENEMY", "PLAYER_SPAWN", "GOAL"]:
                mesh = get_or_load_preview_mesh(ot)
                if not mesh:
                    continue

                if obj.type == 'MESH':
                    obj.data = mesh
                    count += 1
                elif obj.type == 'EMPTY':
                    new_obj = bpy.data.objects.new(obj.name + "_mesh_tmp", mesh)
                    new_obj.matrix_world = obj.matrix_world.copy()
                    for k, v in obj.items():
                        new_obj[k] = v
                    col = obj.users_collection[0] if obj.users_collection else context.collection
                    col.objects.link(new_obj)
                    old_name = obj.name
                    bpy.data.objects.remove(obj, do_unlink=True)
                    new_obj.name = old_name
                    count += 1

        self.report({"INFO"}, f"{count} 個のオブジェクトをモデル表示に更新しました")
        return {"FINISHED"}


# ==========================================
# 3. コライダー設定パネル & 描画
# ==========================================
class MYADDON_OT_add_collider(bpy.types.Operator):
    bl_idname = "myaddon.myaddon_ot_add_collider"
    bl_label = "Collider 追加"
    bl_description = "['collider'] カスタムプロパティを追加します"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        context.object["collider"] = "BOX"
        context.object["collider_center"] = mathutils.Vector((0, 0, 0))
        context.object["collider_size"] = mathutils.Vector((2, 2, 2))
        return {"FINISHED"}


class OBJECT_PT_collider(bpy.types.Panel):
    bl_idname = "OBJECT_PT_collider"
    bl_label = "Collider"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "object"

    def draw(self, context):
        if "collider" in context.object:
            layout = self.layout
            layout.prop(context.object, '["collider"]', text="Type")
            layout.prop(context.object, '["collider_center"]', text="Collider Center")
            layout.prop(context.object, '["collider_size"]', text="Collider Size")
        else:
            self.layout.operator(MYADDON_OT_add_collider.bl_idname, text="Collider 追加")


class DrawCollider:
    handle = None

    @staticmethod
    def draw_colider():
        vertices = {"pos": []}
        indices = []

        ofsets = [
            [-0.5, -0.5, -0.5],
            [+0.5, -0.5, -0.5],
            [-0.5, +0.5, -0.5],
            [+0.5, +0.5, -0.5],
            [-0.5, -0.5, +0.5],
            [+0.5, -0.5, +0.5],
            [-0.5, +0.5, +0.5],
            [+0.5, +0.5, +0.5],
        ]

        for object in bpy.context.scene.objects:
            if "collider" not in object:
                continue

            center = mathutils.Vector((0, 0, 0))
            size = mathutils.Vector((2, 2, 2))

            cc = object.get("collider_center", (0, 0, 0))
            cs = object.get("collider_size", (2, 2, 2))
            center[0] = cc[0]
            center[1] = cc[1]
            center[2] = cc[2]
            size[0] = cs[0]
            size[1] = cs[1]
            size[2] = cs[2]

            start = len(vertices["pos"])
            for ofset in ofsets:
                pos = copy.copy(center)
                pos.x += ofset[0] * size[0]
                pos.y += ofset[1] * size[1]
                pos.z += ofset[2] * size[2]

                pos = object.matrix_world @ pos
                vertices["pos"].append(pos)

            indices.append([start + 0, start + 1])
            indices.append([start + 2, start + 3])
            indices.append([start + 0, start + 2])
            indices.append([start + 1, start + 3])

            indices.append([start + 4, start + 5])
            indices.append([start + 6, start + 7])
            indices.append([start + 4, start + 6])
            indices.append([start + 5, start + 7])
            indices.append([start + 0, start + 4])
            indices.append([start + 1, start + 5])
            indices.append([start + 2, start + 6])
            indices.append([start + 3, start + 7])

        if not vertices["pos"]:
            return

        shader = gpu.shader.from_builtin("UNIFORM_COLOR")
        batch = gpu_extras.batch.batch_for_shader(shader, "LINES", vertices, indices=indices)

        color = [0.5, 1.0, 1.0, 1.0]
        shader.bind()
        shader.uniform_float("color", color)
        batch.draw(shader)


# ==========================================
# 4. ビヘイビアツリー操作
# ==========================================
class MYADDON_OT_add_behavior_tree(bpy.types.Operator):
    bl_idname = "myaddon.myaddon_ot_add_behavior_tree"
    bl_label = "Behavior Tree 追加 ＆ 自動セット"
    bl_description = "['behavior_tree'] プロパティを追加し、存在するツリーを自動選択します"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        target_tree = ""
        for tree in bpy.data.node_groups:
            if tree.bl_idname == 'BehaviorTreeType':
                target_tree = tree.name
                break

        context.object["behavior_tree"] = target_tree
        if target_tree:
            self.report({'INFO'}, f"ツリー '{target_tree}' を自動選択しました。")
        else:
            self.report({'WARNING'}, "BehaviorTreeノードグループが見つかりません。新規作成してください。")
        return {"FINISHED"}


class MYADDON_OT_open_behavior_editor_window(bpy.types.Operator):
    bl_idname = "myaddon.open_behavior_editor_window"
    bl_label = "エディタウィンドウを開く"
    bl_description = "ビヘイビアツリーを編集するための新しいウィンドウを開きます"

    def execute(self, context):
        bpy.ops.wm.window_new()
        new_window = context.window_manager.windows[-1]
        for area in new_window.screen.areas:
            area.type = 'NODE_EDITOR'
            for space in area.spaces:
                if space.type == 'NODE_EDITOR':
                    space.tree_type = 'BehaviorTreeType'
                    obj = context.object
                    if obj and "behavior_tree" in obj and obj["behavior_tree"] in bpy.data.node_groups:
                        space.node_tree = bpy.data.node_groups[obj["behavior_tree"]]
            break
        return {'FINISHED'}


class OBJECT_PT_behavior_tree(bpy.types.Panel):
    bl_idname = "OBJECT_PT_behavior_tree"
    bl_label = "Behavior Tree"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "object"

    def draw(self, context):
        layout = self.layout
        if "behavior_tree" in context.object:
            layout.prop_search(context.object, '["behavior_tree"]', bpy.data, "node_groups", text="Tree")
            tree_name = context.object.get("behavior_tree", "")
            if not tree_name:
                layout.label(text="⚠ ツリーが未選択です", icon='ERROR')
            elif tree_name not in bpy.data.node_groups:
                layout.label(text="⚠ 選択したツリーが見つかりません", icon='ERROR')

            layout.separator()
            layout.operator(MYADDON_OT_open_behavior_editor_window.bl_idname, text="エディタを開く", icon='NODETREE')
        else:
            layout.operator(MYADDON_OT_add_behavior_tree.bl_idname, text="Behavior Tree 追加 ＆ 自動セット")


# ==========================================
# 5. オブジェクト生成 & ファイル名パネル
# ==========================================
class MYADDON_OT_add_filwname(bpy.types.Operator):
    bl_idname = "myaddon.myaddon_ot_add_filename"
    bl_label = "FileName 追加"
    bl_description = "['file_name'] カスタムプロパティを追加します"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        context.object["file_name"] = ""
        return {"FINISHED"}


class MYADDON_OT_ICO_sphere(bpy.types.Operator):
    bl_idname = "myaddon.myaddon_ot_create_object"
    bl_label = "ICO球生成"
    bl_description = "ICO球を生成します"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        bpy.ops.mesh.primitive_ico_sphere_add()
        return {'FINISHED'}


# ==========================================
# 6. シーン出力（エクスポート: Blender座標 -> ゲーム座標）
# ==========================================
class MYADDON_OT_export_scene(bpy.types.Operator, bpy_extras.io_utils.ExportHelper):
    bl_idname = "myaddon.myaddon_ot_export_scene"
    bl_label = "シーン出力"
    bl_description = "シーン情報をExportする"
    filename_ext = ".json"

    convert_to_game_coords: bpy.props.BoolProperty(
        name="ゲーム座標系に変換 (Blender -> Game)",
        description="Blender座標系(Z-up)からゲーム座標系(Y-up)に変換して出力します",
        default=True,
    )

    def draw(self, context):
        self.layout.prop(self, "convert_to_game_coords")

    def execute(self, context):
        print("--- シーン情報Export開始 (ゲーム座標系変換: " + ("ON" if self.convert_to_game_coords else "OFF") + ") ---")
        self.export_json()
        print("--- シーン情報Export完了 ---")
        if self.convert_to_game_coords:
            self.report({'INFO'}, "シーン情報をゲーム座標系(Y-up: X->X, Y->Z, Z->Y)に変換してExportしました！")
        else:
            self.report({'INFO'}, "シーン情報をExportしました (Blender座標のまま)")
        return {'FINISHED'}

    def export_json(self):
        json_object_root = dict()
        json_object_root["name"] = "scene"
        json_object_root["coordinate_system"] = "GAME" if self.convert_to_game_coords else "BLENDER"
        json_object_root["objects"] = list()

        for object in bpy.context.scene.objects:
            if object.parent:
                continue
            self.parse_scene_recursive_json(json_object_root["objects"], object, 0)

        json_text = json.dumps(json_object_root, ensure_ascii=False, cls=json.JSONEncoder, indent=4)
        with open(self.filepath, 'wt', encoding='utf-8') as file:
            file.write(json_text)

    def parse_scene_recursive_json(self, data_parent, object, level):
        json_object = dict()
        # ENEMY, PLAYER_SPAWN, GOAL は Blender 上でメッシュ表示されていてもゲーム互換のため EMPTY として出力
        if object.get("object_type") in ["ENEMY", "PLAYER_SPAWN", "GOAL"]:
            json_object["type"] = "EMPTY"
        else:
            json_object["type"] = object.type
        json_object["name"] = object.name

        # ワールドトランスフォームを取得（親子のズレを防止）
        trans, rot, scale = object.matrix_world.decompose()
        rot = rot.to_euler()
        deg_x = math.degrees(rot.x)
        deg_y = math.degrees(rot.y)
        deg_z = math.degrees(rot.z)

        transform = dict()
        if self.convert_to_game_coords:
            # Blender (X右, Y奥, Z上) -> ゲーム (X右, Y上, Z奥)
            # YとZを入れ替え、X回転の符号を反転
            transform["translation"] = [round(trans.x, 4), round(trans.z, 4), round(trans.y, 4)]
            transform["rotation"]    = [round(-deg_x, 4),  round(deg_z, 4),   round(deg_y, 4)]
            transform["scaling"]     = [round(scale.x, 4), round(scale.z, 4), round(scale.y, 4)]
            print(f"  [Export] {object.name}: Blender({trans.x:.2f}, {trans.y:.2f}, {trans.z:.2f}) -> Game({trans.x:.2f}, {trans.z:.2f}, {trans.y:.2f})")
        else:
            transform["translation"] = [round(trans.x, 4), round(trans.y, 4), round(trans.z, 4)]
            transform["rotation"]    = [round(deg_x, 4),   round(deg_y, 4),   round(deg_z, 4)]
            transform["scaling"]     = [round(scale.x, 4), round(scale.y, 4), round(scale.z, 4)]

        json_object["transform"] = transform

        if "object_type" in object: json_object["object_type"] = object["object_type"]
        if "disabled" in object:    json_object["disabled"]    = bool(object["disabled"])
        if "file_name" in object:   json_object["file_name"]   = object["file_name"]
        if "model_dir" in object:   json_object["model_dir"]   = object["model_dir"]
        if "texture" in object:     json_object["texture"]     = object["texture"]
        if "enemy_type" in object:  json_object["enemy_type"]  = object["enemy_type"]
        if "rail_pos" in object:    json_object["rail_pos"]    = list(object["rail_pos"])
        if "loop" in object:        json_object["loop"]        = bool(object["loop"])
        if "event_name" in object:  json_object["event_name"]  = object["event_name"]
        if "fire_mode" in object:   json_object["fire_mode"]   = object["fire_mode"]

        if "collider" in object:
            collider = dict()
            collider["type"] = object["collider"]
            cc = object.get("collider_center", [0, 0, 0])
            cs = object.get("collider_size", [1, 1, 1])
            cc_list = list(cc) if hasattr(cc, "__iter__") else [0, 0, 0]
            cs_list = list(cs) if hasattr(cs, "__iter__") else [1, 1, 1]
            if self.convert_to_game_coords:
                collider["center"] = [cc_list[0], cc_list[2], cc_list[1]]
                collider["size"]   = [cs_list[0], cs_list[2], cs_list[1]]
            else:
                collider["center"] = cc_list
                collider["size"]   = cs_list
            json_object["collider"] = collider

        if object.type == "CURVE" and object.data:
            curve = object.data
            rail_points = []
            is_cyclic = False
            for spline in curve.splines:
                if spline.use_cyclic_u:
                    is_cyclic = True
                if spline.type == 'BEZIER':
                    for bp in spline.bezier_points:
                        co = bp.co
                        hl = bp.handle_left
                        hr = bp.handle_right
                        if self.convert_to_game_coords:
                            pt_co = [co.x, co.z, co.y]
                            pt_hl = [hl.x, hl.z, hl.y]
                            pt_hr = [hr.x, hr.z, hr.y]
                        else:
                            pt_co = [co.x, co.y, co.z]
                            pt_hl = [hl.x, hl.y, hl.z]
                            pt_hr = [hr.x, hr.y, hr.z]
                        rail_points.append({
                            "co": pt_co,
                            "handle_left": pt_hl,
                            "handle_right": pt_hr,
                        })
            if rail_points:
                json_object["rail_points"] = rail_points
                json_object["loop"] = is_cyclic

        data_parent.append(json_object)
        if len(object.children) > 0:
            json_object["children"] = list()
            for child in object.children:
                self.parse_scene_recursive_json(json_object["children"], child, level + 1)


# ==========================================
# プレビューモデル（メッシュ）読み込み & キャッシュ
# ==========================================
_preview_mesh_cache = {}

def get_project_root():
    """プロジェクトのルートディレクトリを取得"""
    # 1. スクリプトの相対パス (Editer の親フォルダ)
    try:
        cur_file = os.path.abspath(__file__)
        editer_dir = os.path.dirname(cur_file)
        cand = os.path.dirname(editer_dir)
        if os.path.exists(os.path.join(cand, "resources")):
            return cand
    except Exception:
        pass

    # 2. 開いている blend ファイルからの探索
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

    # 3. 既知のプロジェクト絶対パス
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
        # Blenderの自動軸変換を行わず、Rawで読み込み
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

    # メッシュデータを複製
    mesh_data = mesh_obj.data.copy()
    if mesh_name:
        mesh_data.name = mesh_name

    # ゲーム座標系 (X右, Y上, Z奥) -> Blender座標系 (X右, Y奥, Z上) への座標軸変換
    # 行列: X' = X, Y' = Z, Z' = Y
    mat_game_to_blender = mathutils.Matrix((
        (1.0, 0.0, 0.0, 0.0),
        (0.0, 0.0, 1.0, 0.0),
        (0.0, 1.0, 0.0, 0.0),
        (0.0, 0.0, 0.0, 1.0)
    ))
    mesh_data.transform(mat_game_to_blender)
    mesh_data.flip_normals()  # 座標入れ替えによる面の裏返りを補正
    mesh_data.update()

    # テクスチャマテリアル適用
    if tex_path and os.path.exists(tex_path):
        mat = create_texture_material(f"Mat_{mesh_data.name}", tex_path)
        if not mesh_data.materials:
            mesh_data.materials.append(mat)
        else:
            mesh_data.materials[0] = mat

    # 一時オブジェクト削除
    for io in imported_objects:
        bpy.data.objects.remove(io, do_unlink=True)

    # 選択状態復元
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
            print(f"  [PreviewModel] ゲーム座標からBlender座標へ変換して読み込み完了: {obj_type} ({obj_path})")
            return mesh_data

    # フォールバック：シンプルな直方体
    mesh_data = bpy.data.meshes.new(mesh_name)
    verts = [(-0.5, -0.5, -0.5), (0.5, -0.5, -0.5), (0.5, 0.5, -0.5), (-0.5, 0.5, -0.5),
             (-0.5, -0.5,  0.5), (0.5, -0.5,  0.5), (0.5, 0.5,  0.5), (-0.5, 0.5,  0.5)]
    faces = [(0, 1, 2, 3), (4, 7, 6, 5), (0, 4, 5, 1), (1, 5, 6, 2), (2, 6, 7, 3), (4, 0, 3, 7)]
    mesh_data.from_pydata(verts, [], faces)
    mesh_data.update()
    _preview_mesh_cache[obj_type] = mesh_data
    return mesh_data


# ==========================================
# 7. シーンロード（インポート: ゲーム座標 -> Blender座標）
# ==========================================
def _import_object_recursive(objects_json, parent=None, clear_existing=True, convert_coords=True):
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
                            print(f"  [ModelLoad] 地形・メッシュをゲーム座標からBlender座標に変換して読み込み: {name} ({p})")
                            break

            if loaded_mesh:
                blender_obj = bpy.data.objects.new(name, loaded_mesh)
            else:
                mesh_data = bpy.data.meshes.new(name)
                # キューブ頂点を生成
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


# ==========================================
# 7. レールベジェ曲線計算 & スナップユーティリティ
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


# ==========================================
# リアルタイム・マグネット吸着ハンドラー
# ==========================================
_is_magnet_snapping = False

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

    # 対象：rail_pos を持つオブジェクト、または ENEMY / PLAYER_SPAWN / GOAL
    obj_type = active_obj.get("object_type", "")
    is_rail_obj = ("rail_pos" in active_obj) or (obj_type in ["ENEMY", "PLAYER_SPAWN", "GOAL"])
    if not is_rail_obj:
        return

    # オブジェクト自身のトランスフォームが更新されたか判定
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

    # 水平距離 (X, Y) をチェック
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


class OBJECT_PT_rail_settings(bpy.types.Panel):
    """レール進行度設定パネル"""
    bl_idname = "OBJECT_PT_rail_settings"
    bl_label = "Rail Position Settings"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "object"

    def draw(self, context):
        layout = self.layout
        obj = context.object
        scene = context.scene
        if not obj:
            return

        rail = find_target_rail_object(obj.get("rail_target"))
        rail_name = rail.name if rail else "未検出"

        row = layout.row()
        row.label(text=f"参照レール: {rail_name}", icon='CURVE_DATA')

        # 🧲 マグネット自動吸着設定
        box_magnet = layout.box()
        box_magnet.label(text="🧲 レール自動吸着 (近づくと吸着):", icon='SNAP_ON')
        row = box_magnet.row()
        row.prop(scene, "rail_magnet_snap", text="自動吸着モード")
        if scene.rail_magnet_snap:
            box_magnet.prop(scene, "rail_magnet_distance", text="吸着距離 (m)")
            box_magnet.prop(scene, "rail_magnet_keep_height", text="現在の高さを維持")

        layout.separator()

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


# ==========================================
# 8. オブジェクトプロパティ FileName パネル
# ==========================================
class OBJECT_PT_file_name(bpy.types.Panel):
    bl_idname = "OBJECT_PT_file_name"
    bl_label = "FileName"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "object"

    def draw(self, context):
        self.layout.operator(MYADDON_OT_stretch_vertex.bl_idname)
        self.layout.operator(MYADDON_OT_ICO_sphere.bl_idname, text=MYADDON_OT_ICO_sphere.bl_label)
        self.layout.operator(MYADDON_OT_export_scene.bl_idname, text=MYADDON_OT_export_scene.bl_label)
        self.layout.operator(MYADDON_OT_import_scene.bl_idname, text=MYADDON_OT_import_scene.bl_label, icon='IMPORT')

        if "file_name" in context.object:
            self.layout.prop(context.object, '["file_name"]', text=self.bl_label)
        else:
            self.layout.operator(MYADDON_OT_add_filwname.bl_idname, text=MYADDON_OT_add_filwname.bl_label)


# ==========================================
# 9. 最上部メニューバー MyMenu
# ==========================================
class TOPBAR_MT_my_menu(bpy.types.Menu):
    bl_idname = "TOPBAR_MT_my_menu"
    bl_label = "MyMenu"
    bl_description = "拡張メニュー by " + bl_info["author"]

    def draw(self, context):
        self.layout.operator(MYADDON_OT_stretch_vertex.bl_idname, text=MYADDON_OT_stretch_vertex.bl_label)
        self.layout.operator(MYADDON_OT_ICO_sphere.bl_idname, text=MYADDON_OT_ICO_sphere.bl_label)
        self.layout.operator(MYADDON_OT_export_scene.bl_idname, text=MYADDON_OT_export_scene.bl_label)
        self.layout.operator(MYADDON_OT_import_scene.bl_idname, text=MYADDON_OT_import_scene.bl_label)
        self.layout.separator()
        self.layout.operator(MYADDON_OT_apply_all_preview_models.bl_idname, text=MYADDON_OT_apply_all_preview_models.bl_label, icon='MESH_DATA')

    def submenu(self, context):
        self.layout.menu(TOPBAR_MT_my_menu.bl_idname)


# ==========================================
# 登録・解除
# ==========================================
classes = (
    TOPBAR_MT_my_menu,
    MYADDON_OT_export_scene,
    MYADDON_OT_import_scene,
    MYADDON_OT_snap_to_rail,
    MYADDON_OT_calculate_rail_pos,
    MYADDON_OT_add_rail_pos,
    OBJECT_PT_rail_settings,
    MYADDON_OT_apply_all_preview_models,
    MYADDON_OT_ICO_sphere,
    MYADDON_OT_stretch_vertex,
    MYADDON_OT_add_filwname,
    OBJECT_PT_file_name,
    MYADDON_OT_add_collider,
    OBJECT_PT_collider,
    MYADDON_OT_add_behavior_tree,
    MYADDON_OT_open_behavior_editor_window,
    OBJECT_PT_type_settings,
    MYADDON_OT_init_object_type,
    MYADDON_OT_set_object_type,
    OBJECT_PT_behavior_tree,
)

def register():
    for cls in classes:
        try:
            bpy.utils.register_class(cls)
        except ValueError:
            pass

    # シーンプロパティ（マグネット吸着）
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

    try:
        bpy.types.TOPBAR_MT_editor_menus.remove(TOPBAR_MT_my_menu.submenu)
    except Exception:
        pass
    bpy.types.TOPBAR_MT_editor_menus.append(TOPBAR_MT_my_menu.submenu)

    if DrawCollider.handle is None:
        DrawCollider.handle = bpy.types.SpaceView3D.draw_handler_add(DrawCollider.draw_colider, (), "WINDOW", "POST_VIEW")

    # マグネット吸着ハンドラー登録
    if rail_magnet_snap_handler not in bpy.app.handlers.depsgraph_update_post:
        bpy.app.handlers.depsgraph_update_post.append(rail_magnet_snap_handler)

    print("レベルエディタが有効化されました。（レール自動吸着対応）")

def unregister():
    # マグネット吸着ハンドラー解除
    if rail_magnet_snap_handler in bpy.app.handlers.depsgraph_update_post:
        bpy.app.handlers.depsgraph_update_post.remove(rail_magnet_snap_handler)

    # シーンプロパティ削除
    for prop in ("rail_magnet_snap", "rail_magnet_distance", "rail_magnet_keep_height"):
        if hasattr(bpy.types.Scene, prop):
            delattr(bpy.types.Scene, prop)

    if DrawCollider.handle:
        bpy.types.SpaceView3D.draw_handler_remove(DrawCollider.handle, "WINDOW")
        DrawCollider.handle = None
    try:
        bpy.types.TOPBAR_MT_editor_menus.remove(TOPBAR_MT_my_menu.submenu)
    except Exception:
        pass
    for cls in reversed(classes):
        try:
            bpy.utils.unregister_class(cls)
        except Exception:
            pass
    print("レベルエディタが無効化されました。")

if __name__ == "__main__":
    try:
        unregister()
    except Exception:
        pass
    register()
