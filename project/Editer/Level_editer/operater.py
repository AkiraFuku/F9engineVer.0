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


class OBJECT_PT_enemy_spawn_settings(bpy.types.Panel):
    bl_idname = "OBJECT_PT_enemy_spawn_settings"
    bl_label = "Enemy Spawn Settings"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "object"

    @classmethod
    def poll(cls, context):
        obj = context.object
        return obj and obj.get("object_type") == "ENEMY"

    def draw(self, context):
        layout = self.layout
        obj = context.object

        if "spawn_mode" not in obj:
            obj["spawn_mode"] = "TRIGGER_SPAWN"
        if "spawn_distance" not in obj:
            obj["spawn_distance"] = 25.0

        box = layout.box()
        box.label(text="エネミー出現方式:", icon='OUTLINER_OB_ARMATURE')
        col = box.column(align=True)
        col.prop(obj, '["spawn_mode"]', text="出現方式")
        col.prop(obj, '["spawn_distance"]', text="出現感知距離 (m)")

        box_info = layout.box()
        mode = obj.get("spawn_mode", "TRIGGER_SPAWN")
        dist = obj.get("spawn_distance", 25.0)
        if mode == "TRIGGER_SPAWN":
            box_info.label(text=f"📷 プレイヤー手前 {dist:.1f}m で自動スポーン", icon='INFO')
            box_info.label(text="※ 一度出現したらトリガーは機能停止します")
        else:
            box_info.label(text="🏁 ステージ開始時に最初から配置", icon='INFO')





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

        # プレハブブロック手動配置
        box_prefab = self.layout.box()
        box_prefab.label(text="📦 プレハブブロック手動配置:", icon='PACKAGE')
        row_p1 = box_prefab.row(align=True)
        op_std = row_p1.operator(MYADDON_OT_add_prefab_block.bl_idname, text="標準足場", icon='MESH_CUBE')
        op_std.prefab_type = 'STANDARD'
        op_ow = row_p1.operator(MYADDON_OT_add_prefab_block.bl_idname, text="すり抜け足場", icon='RESTRICT_VIEW_OFF')
        op_ow.prefab_type = 'ONEWAY'
        row_p2 = box_prefab.row(align=True)
        op_isl = row_p2.operator(MYADDON_OT_add_prefab_block.bl_idname, text="浮島", icon='MATSPHERE')
        op_isl.prefab_type = 'ISLAND'
        op_stair = row_p2.operator(MYADDON_OT_add_prefab_block.bl_idname, text="階段ステップ", icon='MOD_BEVEL')
        op_stair.prefab_type = 'STAIR_STEP'
        row_p3 = box_prefab.row(align=True)
        op_deco = row_p3.operator(MYADDON_OT_add_prefab_block.bl_idname, text="🎨 背景装飾キューブ (無判定)", icon='COLOR')
        op_deco.prefab_type = 'DECO_CUBE'

        # 地形変形・スカルプト・多角柱彫り込み
        try:
            from .terrain_generator import (
                MYADDON_OT_enter_terrain_sculpt,
                MYADDON_OT_deform_terrain_to_rail,
                MYADDON_OT_carve_prism_depression
            )
        except ImportError:
            from terrain_generator import (
                MYADDON_OT_enter_terrain_sculpt,
                MYADDON_OT_deform_terrain_to_rail,
                MYADDON_OT_carve_prism_depression
            )
        try:
            from .road_generator import MYADDON_OT_generate_road_along_rail
        except ImportError:
            from road_generator import MYADDON_OT_generate_road_along_rail

        box_terrain = self.layout.box()
        box_terrain.label(text="⛰️ 地面メッシュ変形 ＆ 造形:", icon='MOD_OCEAN')
        row_t1 = box_terrain.row(align=True)
        row_t1.operator(MYADDON_OT_deform_terrain_to_rail.bl_idname, text="レール沿いに地面を変形して道をつくる", icon='MOD_SMOOTH')
        row_t2 = box_terrain.row(align=True)
        row_t2.operator(MYADDON_OT_enter_terrain_sculpt.bl_idname, text="なぞって地形変形 (スカルプト)", icon='SCULPTMODE_HLT')
        row_t2.operator(MYADDON_OT_generate_road_along_rail.bl_idname, text="（補助）独立道路メッシュ", icon='CURVE_PATH')
        row_t3 = box_terrain.row(align=True)
        row_t3.operator(MYADDON_OT_carve_prism_depression.bl_idname, text="面を多角柱に押し下げる (3Dカーソル位置)", icon='MOD_SOLIDIFY')

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


class MYADDON_OT_add_prefab_block(bpy.types.Operator):
    bl_idname = "myaddon.add_prefab_block"
    bl_label = "プレハブブロック配置"
    bl_description = "指定したプレハブ（標準足場、すり抜け足場、浮島、階段ステップ、背景装飾）を3Dカーソル位置に配置します"
    bl_options = {"REGISTER", "UNDO"}

    prefab_type: bpy.props.EnumProperty(
        name="プレハブ種別",
        items=[
            ('STANDARD', "標準足場 (Standard)", "標準の足場ブロック"),
            ('ONEWAY', "すり抜け足場 (OneWay)", "下から飛び乗れる薄型足場"),
            ('ISLAND', "浮島 (Floating Island)", "広めの浮遊島ベース"),
            ('STAIR_STEP', "階段ステップ (Stair Step)", "段差・階段用ステップ"),
            ('DECO_CUBE', "背景装飾キューブ (Deco Cube)", "背景に配置する当たり判定なしの装飾キューブ"),
        ],
        default='STANDARD'
    )

    def execute(self, context):
        cursor_loc = context.scene.cursor.location

        # プレハブ定義テーブル
        configs = {
            'STANDARD': {
                'name': 'StandardBlock',
                'prefab_id': 'standard_block',
                'size': (3.0, 3.6, 1.0),
                'texture': 'resources/uvChecker.png',
                'is_oneway': False,
                'is_collision': True,
            },
            'ONEWAY': {
                'name': 'OneWayPlatform',
                'prefab_id': 'oneway_platform',
                'size': (3.5, 2.0, 0.25),
                'texture': 'resources/uvChecker.png',
                'is_oneway': True,
                'is_collision': True,
            },
            'ISLAND': {
                'name': 'FloatingIsland',
                'prefab_id': 'floating_island',
                'size': (6.5, 5.0, 1.5),
                'texture': 'resources/grass.png',
                'is_oneway': False,
                'is_collision': True,
            },
            'STAIR_STEP': {
                'name': 'StairStep',
                'prefab_id': 'stair_step',
                'size': (4.0, 2.0, 0.5),
                'texture': 'resources/grass.png',
                'is_oneway': False,
                'is_collision': True,
            },
            'DECO_CUBE': {
                'name': 'DecoCube',
                'prefab_id': 'deco_cube',
                'size': (2.0, 2.0, 2.0),
                'texture': 'resources/grass.png',
                'is_oneway': False,
                'is_collision': False, # 当たり判定なし
            },
        }

        cfg = configs.get(self.prefab_type, configs['STANDARD'])
        size = cfg['size']

        # 単位キューブ形状（±0.5）でメッシュを生成し、オブジェクトの scale に寸法を反映
        mesh = bpy.data.meshes.new(f"Mesh_{cfg['name']}")
        obj = bpy.data.objects.new(cfg['name'], mesh)
        context.collection.objects.link(obj)

        verts = [
            (-0.5, -0.5, -0.5), (0.5, -0.5, -0.5), (0.5, 0.5, -0.5), (-0.5, 0.5, -0.5),
            (-0.5, -0.5, 0.5),  (0.5, -0.5, 0.5),  (0.5, 0.5, 0.5),  (-0.5, 0.5, 0.5),
        ]
        faces = [
            (0, 1, 2, 3), (4, 7, 6, 5),
            (0, 4, 5, 1), (1, 5, 6, 2),
            (2, 6, 7, 3), (3, 7, 4, 0),
        ]
        mesh.from_pydata(verts, [], faces)
        mesh.update()

        obj.location = cursor_loc.copy()
        obj.scale = (size[0], size[1], size[2])

        # ゲームエンジン用メタデータ
        obj["object_type"] = "BLOCK"
        obj["file_name"] = "box"
        obj["texture"] = cfg['texture']
        obj["prop_prefab_id"] = cfg['prefab_id']
        if cfg['is_oneway']:
            obj["prop_is_oneway"] = "true"
        if not cfg['is_collision']:
            obj["prop_is_collision"] = "false"
            obj["prop_collision"] = "false"

        context.view_layer.objects.active = obj
        obj.select_set(True)
        col_text = "（当たり判定なし）" if not cfg['is_collision'] else ""
        self.report({"INFO"}, f"プレハブ '{obj.name}' [{cfg['prefab_id']}]{col_text} を配置しました")
        return {"FINISHED"}
