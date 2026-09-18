import bpy

bl_info={
    "name": "レベルエディタ",
    "author": "Fuku Akira",
    "version": (1,1),
    "blender": (3,3,1),
    "location": "",
    "description": "レベルエディタ（BT出力自動補正機能付き）",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Object"
}
from .stretch_vertex import MYADDON_OT_stretch_vertex
from .object_type import OBJECT_PT_type_settings
from .object_type import MYADDON_OT_init_object_type
from .object_type import MYADDON_OT_set_object_type
from .colider import MYADDON_OT_add_collider
from .colider import OBJECT_PT_collider
from .colider import DrawCollider
from .operater import MYADDON_OT_add_behavior_tree
from .operater import MYADDON_OT_open_behavior_editor_window
from .operater import OBJECT_PT_behavior_tree
from .operater import OBJECT_PT_file_name
from .operater import MYADDON_OT_add_filwname
from .operater import MYADDON_OT_ICO_sphere
from .export_scene import MYADDON_OT_export_scene




class TOPBAR_MT_my_menu(bpy.types.Menu):
    bl_idname = "TOPBAR_MT_my_menu"
    bl_label = "MyMenu"
    bl_description = "拡張メニュー by " + bl_info["author"]

    def draw(self, context):
        self.layout.operator(MYADDON_OT_stretch_vertex.bl_idname, text=MYADDON_OT_stretch_vertex.bl_label)
        self.layout.operator(MYADDON_OT_ICO_sphere.bl_idname, text=MYADDON_OT_ICO_sphere.bl_label)
        self.layout.operator(MYADDON_OT_export_scene.bl_idname, text=MYADDON_OT_export_scene.bl_label)
         
    def submenu(self, context):
        self.layout.menu(TOPBAR_MT_my_menu.bl_idname)


classes = (
    TOPBAR_MT_my_menu,
    MYADDON_OT_export_scene,
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
        bpy.utils.register_class(cls)
    bpy.types.TOPBAR_MT_editor_menus.append(TOPBAR_MT_my_menu.submenu)
    DrawCollider.handle = bpy.types.SpaceView3D.draw_handler_add(DrawCollider.draw_colider, (), "WINDOW", "POST_VIEW")
    print("レベルエディタが有効化されました。")

def unregister():
    if DrawCollider.handle:
        bpy.types.SpaceView3D.draw_handler_remove(DrawCollider.handle, "WINDOW")
    bpy.types.TOPBAR_MT_editor_menus.remove(TOPBAR_MT_my_menu.submenu)
   
    for cls in classes:
        bpy.utils.unregister_class(cls)
    print("レベルエディタが無効化されました。")

if __name__ == "__main__":
    try:
        unregister()
    except Exception:
        pass
    register()