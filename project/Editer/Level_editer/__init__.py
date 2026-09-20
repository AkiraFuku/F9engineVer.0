import bpy

bl_info = {
    "name": "レベルエディタ",
    "author": "Fuku Akira",
    "version": (1, 2),
    "blender": (3, 3, 1),
    "location": "TopBar > MyMenu / Object Properties",
    "description": "ステージエディタ（地形グリッド生成・OBJ変換・レール自動吸着・シーン入出力対応）",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Object",
}

from .stretch_vertex import MYADDON_OT_stretch_vertex
from .object_type import (
    OBJECT_PT_type_settings,
    MYADDON_OT_init_object_type,
    MYADDON_OT_set_object_type,
    MYADDON_OT_apply_all_preview_models,
)
from .colider import (
    MYADDON_OT_add_collider,
    OBJECT_PT_collider,
    DrawCollider,
)
from .operater import (
    MYADDON_OT_add_behavior_tree,
    MYADDON_OT_open_behavior_editor_window,
    OBJECT_PT_behavior_tree,
    OBJECT_PT_file_name,
    MYADDON_OT_add_filwname,
    MYADDON_OT_ICO_sphere,
)
from .export_scene import MYADDON_OT_export_scene
from .import_scene import MYADDON_OT_import_scene
from .terrain_generator import (
    MYADDON_OT_generate_terrain,
    MYADDON_OT_convert_obj_to_terrain,
)
from .rail_snap import (
    MYADDON_OT_snap_to_rail,
    MYADDON_OT_calculate_rail_pos,
    MYADDON_OT_add_rail_pos,
    MYADDON_OT_sync_camera_rail,
    MYADDON_OT_unlink_camera_rail,
    OBJECT_PT_rail_settings,
    rail_magnet_snap_handler,
    register_rail_snap,
    unregister_rail_snap,
)


class TOPBAR_MT_my_menu(bpy.types.Menu):
    bl_idname = "TOPBAR_MT_my_menu"
    bl_label = "MyMenu"
    bl_description = "拡張メニュー by " + bl_info["author"]

    def draw(self, context):
        self.layout.operator(MYADDON_OT_stretch_vertex.bl_idname, text=MYADDON_OT_stretch_vertex.bl_label)
        self.layout.operator(MYADDON_OT_ICO_sphere.bl_idname, text=MYADDON_OT_ICO_sphere.bl_label)
        self.layout.operator(MYADDON_OT_export_scene.bl_idname, text=MYADDON_OT_export_scene.bl_label)
        self.layout.operator(MYADDON_OT_import_scene.bl_idname, text=MYADDON_OT_import_scene.bl_label, icon='IMPORT')
        self.layout.separator()
        self.layout.operator(MYADDON_OT_apply_all_preview_models.bl_idname, text=MYADDON_OT_apply_all_preview_models.bl_label, icon='MESH_DATA')
        self.layout.separator()
        self.layout.operator(MYADDON_OT_generate_terrain.bl_idname, text=MYADDON_OT_generate_terrain.bl_label, icon='MESH_GRID')
        self.layout.operator(MYADDON_OT_convert_obj_to_terrain.bl_idname, text=MYADDON_OT_convert_obj_to_terrain.bl_label, icon='MOD_OCEAN')
        self.layout.separator()
        self.layout.operator(MYADDON_OT_sync_camera_rail.bl_idname, text=MYADDON_OT_sync_camera_rail.bl_label, icon='CAMERA_DATA')

def draw_my_menu(self, context):
    self.layout.menu(TOPBAR_MT_my_menu.bl_idname)


classes = (
    TOPBAR_MT_my_menu,
    MYADDON_OT_export_scene,
    MYADDON_OT_import_scene,
    MYADDON_OT_ICO_sphere,
    MYADDON_OT_stretch_vertex,
    MYADDON_OT_add_filwname,
    OBJECT_PT_file_name,
    MYADDON_OT_add_collider,
    OBJECT_PT_collider,
    MYADDON_OT_add_behavior_tree,
    MYADDON_OT_open_behavior_editor_window,
    OBJECT_PT_behavior_tree,
    OBJECT_PT_type_settings,
    MYADDON_OT_init_object_type,
    MYADDON_OT_set_object_type,
    MYADDON_OT_apply_all_preview_models,
    MYADDON_OT_generate_terrain,
    MYADDON_OT_convert_obj_to_terrain,
    MYADDON_OT_snap_to_rail,
    MYADDON_OT_calculate_rail_pos,
    MYADDON_OT_add_rail_pos,
    MYADDON_OT_sync_camera_rail,
    MYADDON_OT_unlink_camera_rail,
    OBJECT_PT_rail_settings,
)


def register():
    for cls in classes:
        try:
            bpy.utils.register_class(cls)
        except ValueError:
            pass

    # トップバーメニューに追加
    try:
        bpy.types.TOPBAR_MT_editor_menus.remove(draw_my_menu)
    except Exception:
        pass
    try:
        bpy.types.TOPBAR_MT_editor_menus.append(draw_my_menu)
    except Exception:
        pass

    # 3Dビューポートのヘッダーメニューにも追加（見失い防止）
    try:
        bpy.types.VIEW3D_MT_editor_menus.remove(draw_my_menu)
    except Exception:
        pass
    try:
        bpy.types.VIEW3D_MT_editor_menus.append(draw_my_menu)
    except Exception:
        pass

    if DrawCollider.handle is None:
        DrawCollider.handle = bpy.types.SpaceView3D.draw_handler_add(
            DrawCollider.draw_colider, (), "WINDOW", "POST_VIEW"
        )

    register_rail_snap()
    print("レベルエディタが有効化されました。（地形グリッド・レール自動吸着対応）")


def unregister():
    unregister_rail_snap()

    if DrawCollider.handle:
        bpy.types.SpaceView3D.draw_handler_remove(DrawCollider.handle, "WINDOW")
        DrawCollider.handle = None

    try:
        bpy.types.TOPBAR_MT_editor_menus.remove(draw_my_menu)
    except Exception:
        pass
    try:
        bpy.types.VIEW3D_MT_editor_menus.remove(draw_my_menu)
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