import bpy
# ==========================================
# 1. オブジェクトタイプ設定用
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
            row.operator("myaddon.set_object_type", text="PROP(背景)にする").target_type = "PROP"

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

    target_type: bpy.props.StringProperty() # type: ignore

    def execute(self, context):
        context.object["object_type"] = self.target_type
        return {"FINISHED"}
