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

        if not obj:
            return

        if "object_type" not in obj:
            layout.operator("myaddon.init_object_type", text="オブジェクトタイプを初期化")
        else:
            row = layout.row(align=True)
            row.label(text=f"現在のタイプ: {obj['object_type']}", icon='OBJECT_DATA')
            
            layout.separator()
            if obj.type == 'CURVE':
                box_rail = layout.box()
                box_rail.label(text="レール種別設定:")
                row = box_rail.row(align=True)
                row.operator("myaddon.set_object_type", text="STAGE_RAIL (ステージレール)", icon='CURVE_BEZCURVE').target_type = "STAGE_RAIL"
                row.operator("myaddon.set_object_type", text="CAMERA_RAIL (カメラレール)", icon='CAMERA_DATA').target_type = "CAMERA_RAIL"
            else:
                box_types = layout.box()
                box_types.label(text="オブジェクト種別設定:")
                row1 = box_types.row(align=True)
                row1.operator("myaddon.set_object_type", text="ENEMY", icon='COMMUNITY').target_type = "ENEMY"
                row1.operator("myaddon.set_object_type", text="PLAYER", icon='USER').target_type = "PLAYER_SPAWN"
                row1.operator("myaddon.set_object_type", text="GOAL", icon='CHECKMARK').target_type = "GOAL"
                row2 = box_types.row(align=True)
                row2.operator("myaddon.set_object_type", text="BLOCK (地形ブロック)", icon='MESH_CUBE').target_type = "BLOCK"
                row2.operator("myaddon.set_object_type", text="PROP (背景)", icon='SCENE_DATA').target_type = "PROP"

        if obj.type != 'CURVE':
            layout.separator()
            layout.operator("myaddon.apply_all_preview_models", text="全Enemy/Playerをモデル表示に更新", icon='MESH_DATA')

        # メッシュオブジェクトの場合、地形グリッドへの変換ボタンを表示
        if obj and obj.type == 'MESH':
            layout.separator()
            box = layout.box()
            box.label(text="地形グリッド変換:", icon='MOD_OCEAN')
            box.operator("myaddon.convert_obj_to_terrain", text="このメッシュを地形グリッドに変換", icon='MESH_GRID')


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
            try:
                from .import_scene import get_or_load_preview_mesh
                mesh = get_or_load_preview_mesh(self.target_type)
            except Exception:
                try:
                    import import_scene
                    mesh = import_scene.get_or_load_preview_mesh(self.target_type)
                except Exception:
                    mesh = None

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
        try:
            from .import_scene import get_or_load_preview_mesh, _preview_mesh_cache
            _preview_mesh_cache.clear()
        except Exception:
            try:
                import import_scene
                import_scene._preview_mesh_cache.clear()
                get_or_load_preview_mesh = import_scene.get_or_load_preview_mesh
            except Exception:
                get_or_load_preview_mesh = None

        # 既存のプレビューメッシュを削除して再読み込みを保証
        for ot in ["ENEMY", "PLAYER_SPAWN", "GOAL"]:
            m_name = f"Mesh_Preview_{ot}"
            if m_name in bpy.data.meshes:
                bpy.data.meshes.remove(bpy.data.meshes[m_name], do_unlink=True)

        if not get_or_load_preview_mesh:
            self.report({"WARNING"}, "import_scene モジュールからモデルを読み込めませんでした")
            return {"CANCELLED"}

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


classes = (
    OBJECT_PT_type_settings,
    MYADDON_OT_init_object_type,
    MYADDON_OT_set_object_type,
    MYADDON_OT_apply_all_preview_models,
)
