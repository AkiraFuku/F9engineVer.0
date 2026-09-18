import bpy
import bpy_extras
import json
import math


# ==========================================
# JSON ステージデータ インポーター
# ==========================================

def _import_object_recursive(objects_json, parent=None, clear_existing=True):
    """JSON オブジェクトリストを再帰的に Blender オブジェクトへ変換"""
    for obj_data in objects_json:
        name        = obj_data.get("name", "Unnamed")
        type_str    = obj_data.get("type", "MESH")
        obj_type    = obj_data.get("object_type", "")
        transform   = obj_data.get("transform", {})
        file_name   = obj_data.get("file_name", "")
        texture     = obj_data.get("texture", "")
        disabled    = obj_data.get("disabled", False)
        rail_pos    = obj_data.get("rail_pos", None)
        enemy_type  = obj_data.get("enemy_type", "")
        loop        = obj_data.get("loop", False)
        rail_points = obj_data.get("rail_points", [])
        properties  = obj_data.get("properties", {})
        children    = obj_data.get("children", [])

        # DX 座標系 -> Blender 座標系 逆変換
        tr_raw = transform.get("translation", [0, 0, 0])
        sc_raw = transform.get("scaling",     [1, 1, 1])
        ro_raw = transform.get("rotation",    [0, 0, 0])

        # DX(x,y,z) -> Blender(x,z,y)
        loc   = (tr_raw[0], tr_raw[2], tr_raw[1])
        scale = (sc_raw[0], sc_raw[2], sc_raw[1])
        rot_rad = (
            -ro_raw[0],
             ro_raw[2],
             ro_raw[1],
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
                bpt.co           = (co[0], co[2], co[1])
                bpt.handle_left  = (hl[0], hl[2], hl[1])
                bpt.handle_right = (hr[0], hr[2], hr[1])
                bpt.handle_left_type  = "FREE"
                bpt.handle_right_type = "FREE"
            blender_obj = bpy.data.objects.new(name, curve_data)
            bpy.context.collection.objects.link(blender_obj)

        # EMPTY
        elif type_str == "EMPTY":
            bpy.ops.object.empty_add(type="PLAIN_AXES", location=loc)
            blender_obj = bpy.context.active_object
            blender_obj.name = name

        # MESH その他
        else:
            bpy.ops.mesh.primitive_cube_add(location=loc)
            blender_obj = bpy.context.active_object
            blender_obj.name = name

        if blender_obj:
            blender_obj.location       = loc
            blender_obj.rotation_euler = rot_rad
            blender_obj.scale          = scale

            # カスタムプロパティ復元
            blender_obj["object_type"] = obj_type
            if file_name:  blender_obj["file_name"]  = file_name
            if texture:    blender_obj["texture"]     = texture
            if disabled:   blender_obj["disabled"]    = disabled
            if enemy_type: blender_obj["enemy_type"]  = enemy_type
            if loop:       blender_obj["loop"]        = loop
            if rail_pos is not None: blender_obj["rail_pos"] = rail_pos
            for k, v in properties.items():
                blender_obj[k] = v

            if parent:
                blender_obj.parent = parent

            if children:
                _import_object_recursive(children, parent=blender_obj,
                                         clear_existing=clear_existing)


class MYADDON_OT_import_scene(bpy.types.Operator, bpy_extras.io_utils.ImportHelper):
    bl_idname    = "myaddon.import_scene"
    bl_label     = "JSONステージ読み込み"
    bl_description = "ステージ JSON ファイルを読み込んでシーンに配置します"

    filename_ext = ".json"
    filter_glob: bpy.props.StringProperty(default="*.json", options={"HIDDEN"})

    clear_scene: bpy.props.BoolProperty(
        name="既存オブジェクトを上書き",
        description="同名オブジェクトが存在する場合に削除して再作成します",
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

        _import_object_recursive(objects, parent=None, clear_existing=self.clear_scene)
        self.report({"INFO"}, f"{len(objects)} 個のトップレベルオブジェクトを配置しました")
        return {"FINISHED"}

    def draw(self, context):
        self.layout.prop(self, "clear_scene")
