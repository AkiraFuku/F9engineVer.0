import bpy

try:
    from .stretch_vertex import MYADDON_OT_stretch_vertex
    from .export_scene import MYADDON_OT_export_scene
except ImportError:
    from stretch_vertex import MYADDON_OT_stretch_vertex
    from export_scene import MYADDON_OT_export_scene
# ==========================================
# 3. ビヘイビアツリー設定パネル＆自動セットオペレーター（強化）
# ==========================================
class MYADDON_OT_add_behavior_tree(bpy.types.Operator):
    bl_idname = "myaddon.myaddon_ot_add_behavior_tree"
    bl_label = "Behavior Tree 追加 ＆ 自動セット"
    bl_description = "['behavior_tree'] プロパティを追加し、存在するツリーを自動選択します"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        # 最初に見つかった Behavioral Tree を割り当てる
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
                    # アクティブなオブジェクトに割り当たっているツリーを自動表示
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

    @classmethod
    def poll(cls, context):
        # エネミーまたはビヘイビアツリー持ちオブジェクトのみ表示
        obj = context.object
        if not obj or obj.type == 'CURVE':
            return False
        return (obj.get("object_type") == "ENEMY") or ("behavior_tree" in obj)

    def draw(self, context):
        layout = self.layout
        if "behavior_tree" in context.object:
            layout.prop_search(context.object, '["behavior_tree"]', bpy.data, "node_groups", text="Tree")
            
            # 選択中のツリーに対する注意事項表示
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
# 5. 各種パネル・オペレーター
# ==========================================
class OBJECT_PT_file_name(bpy.types.Panel):
    bl_idname = "OBJECT_PT_file_name"
    bl_label = "FileName & Stage Blocks"
    bl_space_type = "PROPERTIES"
    bl_region_type= "WINDOW"
    bl_context = "object"

    @classmethod
    def poll(cls, context):
        # レール（CURVE）には非表示
        obj = context.object
        return obj is not None and obj.type != 'CURVE'

    def draw(self, context):
        try:
            from .import_scene import MYADDON_OT_import_scene
        except ImportError:
            from import_scene import MYADDON_OT_import_scene
        self.layout.operator(MYADDON_OT_stretch_vertex.bl_idname)
        self.layout.operator(MYADDON_OT_ICO_sphere.bl_idname, text=MYADDON_OT_ICO_sphere.bl_label)
        self.layout.operator(MYADDON_OT_export_scene.bl_idname, text=MYADDON_OT_export_scene.bl_label)
        self.layout.operator(MYADDON_OT_import_scene.bl_idname, text=MYADDON_OT_import_scene.bl_label, icon='IMPORT')

        if "file_name" in context.object:
            self.layout.prop(context.object, '["file_name"]', text=self.bl_label)
        else:
            self.layout.operator(MYADDON_OT_add_filwname.bl_idname, text=MYADDON_OT_add_filwname.bl_label)

        # 傾斜・段差ブロックの追加
        self.layout.separator()
        box_stage = self.layout.box()
        box_stage.label(text="地形・段差ブロック追加:", icon='MESH_GRID')
        row_stage = box_stage.row(align=True)
        row_stage.operator(MYADDON_OT_add_slope_block.bl_idname, text="傾斜スロープ追加", icon='MOD_SOLIDIFY')
        row_stage.operator(MYADDON_OT_add_stairs_block.bl_idname, text="階段ブロック追加", icon='MOD_BEVEL')

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
# 6. 傾斜（スロープ）＆階段（段差）ブロック追加オペレーター
# ==========================================

def _get_project_root_dir():
    cur = os.path.abspath(__file__)
    return os.path.dirname(os.path.dirname(os.path.dirname(cur)))

def _load_or_get_stage_mesh(rel_path, mesh_name):
    root = _get_project_root_dir()
    full_path = os.path.join(root, rel_path)
    if not os.path.exists(full_path):
        return None
    try:
        from .import_scene import load_obj_mesh_converted
        return load_obj_mesh_converted(full_path, mesh_name)
    except Exception:
        try:
            import import_scene
            return import_scene.load_obj_mesh_converted(full_path, mesh_name)
        except Exception:
            return None


class MYADDON_OT_add_slope_block(bpy.types.Operator):
    bl_idname = "myaddon.add_slope_block"
    bl_label = "傾斜スロープ追加"
    bl_description = "プレイヤーがスムーズに登り降りできる傾斜スロープ（ウェッジ形状）ブロックを追加します"
    bl_options = {"REGISTER", "UNDO"}

    width: bpy.props.FloatProperty(name="幅 (m)", default=4.0, min=0.5, max=100.0)
    length: bpy.props.FloatProperty(name="奥行き (m)", default=6.0, min=0.5, max=100.0)
    height: bpy.props.FloatProperty(name="高さ (m)", default=2.0, min=0.1, max=100.0)

    def execute(self, context):
        mesh = _load_or_get_stage_mesh("resources/Stagemap/slope.obj", "Mesh_SlopeBlock")
        if not mesh:
            # フォールバックとしてキューブ
            bpy.ops.mesh.primitive_cube_add()
            obj = context.object
            obj.name = "SlopeBlock"
        else:
            obj = bpy.data.objects.new("SlopeBlock", mesh)
            context.collection.objects.link(obj)

        obj["object_type"] = "BLOCK"
        obj["file_name"] = "slope.obj"
        obj["model_dir"] = "resources/Stagemap"
        obj["texture"] = "resources/Stagemap/863603.png"

        # Blender座標系でのスケール (X幅, Y奥行き, Z高さ)
        obj.scale = (self.width / 2.0, self.length / 4.0, self.height / 1.0)
        obj.location = context.scene.cursor.location

        context.view_layer.objects.active = obj
        obj.select_set(True)
        self.report({"INFO"}, f"傾斜スロープ '{obj.name}' (幅:{self.width}m, 奥行:{self.length}m, 高:{self.height}m) を配置しました")
        return {"FINISHED"}


class MYADDON_OT_add_stairs_block(bpy.types.Operator):
    bl_idname = "myaddon.add_stairs_block"
    bl_label = "階段ブロック追加"
    bl_description = "プレイヤーが足元を引っ掛けずに登れる4段ステップ（階段）ブロックを追加します"
    bl_options = {"REGISTER", "UNDO"}

    width: bpy.props.FloatProperty(name="幅 (m)", default=4.0, min=0.5, max=100.0)
    length: bpy.props.FloatProperty(name="奥行き (m)", default=6.0, min=0.5, max=100.0)
    height: bpy.props.FloatProperty(name="高さ (m)", default=1.0, min=0.1, max=100.0)

    def execute(self, context):
        mesh = _load_or_get_stage_mesh("resources/Stagemap/stairs.obj", "Mesh_StairsBlock")
        if not mesh:
            bpy.ops.mesh.primitive_cube_add()
            obj = context.object
            obj.name = "StairsBlock"
        else:
            obj = bpy.data.objects.new("StairsBlock", mesh)
            context.collection.objects.link(obj)

        obj["object_type"] = "BLOCK"
        obj["file_name"] = "stairs.obj"
        obj["model_dir"] = "resources/Stagemap"
        obj["texture"] = "resources/Stagemap/863603.png"

        # Blender座標系でのスケール (X幅, Y奥行き, Z高さ)
        obj.scale = (self.width / 2.0, self.length / 4.0, self.height / 1.0)
        obj.location = context.scene.cursor.location

        context.view_layer.objects.active = obj
        obj.select_set(True)
        self.report({"INFO"}, f"階段ブロック '{obj.name}' (幅:{self.width}m, 奥行:{self.length}m, 高:{self.height}m) を配置しました")
        return {"FINISHED"}
