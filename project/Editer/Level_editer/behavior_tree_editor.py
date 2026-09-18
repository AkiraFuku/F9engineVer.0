import bpy
import math
import bpy_extras
import gpu
import gpu_extras.batch
import copy
import mathutils
#NodeEditor用のモジュールをインポート
import nodeitems_utils
from nodeitems_utils import NodeCategory, NodeItem
#ノードのカテゴリを定義するためのモジュールをインポート

#アドオン情報
bl_info={
    "name": "エネミー行動ツリーエディタ",
    "author": "Fuku Akira",
    "version": (1,1),
    "blender": (3,3,1),
    "location": "",
    "description": "エネミー行動ツリーエディタ",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Object"
}

#登録するクラス
class BehaviorTreeSocket(bpy.types.NodeSocket):
    bl_idname = 'BehaviorTreeSocketType'
    bl_label = "BT Socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.2, 0.8, 0.2, 1.0)

class BehaviorTreeNodeTree(bpy.types.NodeTree):
    bl_idname = 'BehaviorTreeType'
    bl_label = "Behavior Tree Editor"
    bl_icon = 'ACTION'

class BehaviorTreeNodeBase:
    """すべてのBehavior Treeノードのベースクラス"""
    @classmethod
    def poll(cls, ntree):
        return ntree.bl_idname == 'BehaviorTreeType'
        
    def get_export_data(self):
        """エクスポート時にノードの情報を辞書として返す"""
        return {"type": self.__class__.__name__}

    def has_loop(self, visited=None):
        """循環参照（無限ループ）を検知するための再帰関数 (改善点4)"""
        if visited is None:
            visited = set()
        
        if self in visited:
            return True
        
        visited.add(self)
        for output in self.outputs:
            for link in output.links:
                child = link.to_node
                if isinstance(child, BehaviorTreeNodeBase):
                    if child.has_loop(visited):
                        return True
        visited.remove(self)
        return False

class CreateBehaviorTreeWorkspaceOperator(bpy.types.Operator):
    """Behavior Tree用ワークスペースを作成"""
    bl_idname = "workspace.create_behavior_tree"
    bl_label = "Create Behavior Tree Workspace"
    
    def execute(self, context):
        if "Behavior Tree" in bpy.data.workspaces:
            context.window.workspace = bpy.data.workspaces["Behavior Tree"]
            return {'FINISHED'}
            
        bpy.ops.workspace.duplicate()
        ws = context.window.workspace
        ws.name = "Behavior Tree"
        
        for screen in ws.screens:
            for area in screen.areas:
                if area.type == 'VIEW_3D':
                    area.type = 'NODE_EDITOR'
                    for space in area.spaces:
                        if space.type == 'NODE_EDITOR':
                            space.tree_type = 'BehaviorTreeType'
                    break
                    
        return {'FINISHED'}

class BTNode_Root(bpy.types.Node, BehaviorTreeNodeBase):
    bl_idname = 'BTNode_Root'
    bl_label = "Root"
    bl_icon = 'OUTLINER_OB_ARMATURE'

    def init(self, context):
        self.outputs.new('BehaviorTreeSocketType', "Child")

class BTNode_Sequence(bpy.types.Node, BehaviorTreeNodeBase):
    bl_idname = 'BTNode_Sequence'
    bl_label = "Sequence"
    bl_icon = 'SORT_DESC'

    child_count: bpy.props.IntProperty(
        name="Child Count",
        default=2,
        min=1,
        max=16,
        update=lambda self, ctx: self._rebuild_outputs()
    ) # type: ignore

    def _rebuild_outputs(self):
        # 現在のソケット数と設定された数の差分を計算 (改善点2)
        current_len = len(self.outputs)
        target_len = self.child_count
    
        if current_len < target_len:
            # 不足している分だけ新しく追加
            for i in range(current_len, target_len):
                self.outputs.new('BehaviorTreeSocketType', f"Child_{i}")
        elif current_len > target_len:
            # 超過している分だけ末尾から削除
            for i in range(current_len - 1, target_len - 1, -1):
                self.outputs.remove(self.outputs[i])
    
    def init(self, context):
        self.inputs.new('BehaviorTreeSocketType', "Parent")
        self._rebuild_outputs()

    def draw_buttons(self, context, layout):
        layout.prop(self, "child_count")

    def get_export_data(self):
        data = super().get_export_data()
        data["child_count"] = self.child_count
        return data

class BTNode_Selector(bpy.types.Node, BehaviorTreeNodeBase):
    bl_idname = 'BTNode_Selector'
    bl_label = "Selector"
    bl_icon = 'QUESTION'

    child_count: bpy.props.IntProperty(
        name="Child Count",
        default=2,
        min=1,
        max=16,
        update=lambda self, ctx: self._rebuild_outputs()
    ) # type: ignore

    def _rebuild_outputs(self):
        # SelectorノードもSequenceと同様に既存リンクを残す処理に修正 (改善点2)
        current_len = len(self.outputs)
        target_len = self.child_count
    
        if current_len < target_len:
            for i in range(current_len, target_len):
                self.outputs.new('BehaviorTreeSocketType', f"Child_{i}")
        elif current_len > target_len:
            for i in range(current_len - 1, target_len - 1, -1):
                self.outputs.remove(self.outputs[i])

    def init(self, context):
        self.inputs.new('BehaviorTreeSocketType', "Parent")
        self._rebuild_outputs()

    def draw_buttons(self, context, layout):
        layout.prop(self, "child_count")

    def get_export_data(self):
        data = super().get_export_data()
        data["child_count"] = self.child_count
        return data

class BTNode_Inverter(bpy.types.Node, BehaviorTreeNodeBase):
    bl_idname = 'BTNode_Inverter'
    bl_label = "Inverter"
    bl_icon = 'ARROW_LEFTRIGHT'

    def init(self, context):
        self.inputs.new('BehaviorTreeSocketType', "Parent")
        self.outputs.new('BehaviorTreeSocketType', "Child")

class BTNode_Branch(bpy.types.Node, BehaviorTreeNodeBase):
    bl_idname = 'BTNode_Branch'
    bl_label = "Branch"
    bl_icon = 'LIBRARY_DATA_BROKEN'

    # JSON条件テキスト
    condition_json: bpy.props.StringProperty(
        name="Condition JSON",
        description="分岐条件をJSON形式で記述します\n例: {\"condition\": \"hp_less_than\", \"value\": 50}",
        default='{"condition": "", "value": 0}'
    ) # type: ignore

    def init(self, context):
        self.inputs.new('BehaviorTreeSocketType', "Parent")
        self.outputs.new('BehaviorTreeSocketType', "True")
        self.outputs.new('BehaviorTreeSocketType', "False")

    def draw_buttons(self, context, layout):
        layout.label(text="Condition JSON:")
        layout.prop(self, "condition_json", text="")
        
        # JSONバリデーションフィードバック
        try:
            import json
            json.loads(self.condition_json)
            layout.label(text="✓ Valid JSON", icon='CHECKMARK')
        except Exception:
            layout.label(text="⚠ Invalid JSON", icon='ERROR')

    def get_export_data(self):
        import json
        data = super().get_export_data()
        try:
            data["condition"] = json.loads(self.condition_json)
        except Exception:
            data["condition"] = self.condition_json
        return data

class BTNode_CondRandom(bpy.types.Node, BehaviorTreeNodeBase):
    """新しく追加された確率判定用の条件ノード"""
    bl_idname = 'BTNode_CondRandom'
    bl_label = "Random Chance"
    bl_icon = 'QUESTION'

    chance: bpy.props.FloatProperty(
        name="Chance (%)", 
        default=40.0, 
        min=0.0, 
        max=100.0,
        description="成功する確率を設定します"
    ) # type: ignore

    def init(self, context):
        self.inputs.new('BehaviorTreeSocketType', "Parent")
        self.outputs.new('BehaviorTreeSocketType', "If Success")

    def draw_buttons(self, context, layout):
        layout.prop(self, "chance", slider=True)
        
    def get_export_data(self):
        data = super().get_export_data()
        data["chance"] = self.chance / 100.0
        return data

class BTNode_ActionWait(bpy.types.Node, BehaviorTreeNodeBase):
    bl_idname = 'BTNode_ActionWait'
    bl_label = "Wait"
    bl_icon = 'TIME'

    wait_time: bpy.props.FloatProperty(name="Wait Time", default=1.0, min=0.0) # type: ignore

    def init(self, context):
        self.inputs.new('BehaviorTreeSocketType', "Parent")

    def draw_buttons(self, context, layout):
        layout.prop(self, "wait_time")
        
    def get_export_data(self):
        data = super().get_export_data()
        data["wait_time"] = self.wait_time
        return data

class BTNode_ActionMoveTo(bpy.types.Node, BehaviorTreeNodeBase):
    bl_idname = 'BTNode_ActionMoveTo'
    bl_label = "Move To"
    bl_icon = 'CON_LOCLIKE'

    speed: bpy.props.FloatProperty(name="Speed", default=1.0, min=0.0) # type: ignore

    def init(self, context):
        self.inputs.new('BehaviorTreeSocketType', "Parent")

    def draw_buttons(self, context, layout):
        layout.prop(self, "speed")
        
    def get_export_data(self):
        data = super().get_export_data()
        data["speed"] = self.speed
        return data

class BTNode_ActionTurn(bpy.types.Node, BehaviorTreeNodeBase):
    """新しく追加された旋回アクションノード"""
    bl_idname = 'BTNode_ActionTurn'
    bl_label = "Turn"
    bl_icon = 'FILE_REFRESH'

    angle: bpy.props.FloatProperty(
        name="Angle", 
        default=180.0,
        description="旋回する角度（度数法）"
    ) # type: ignore

    def init(self, context):
        self.inputs.new('BehaviorTreeSocketType', "Parent")

    def draw_buttons(self, context, layout):
        layout.prop(self, "angle")
        
    def get_export_data(self):
        data = super().get_export_data()
        data["angle"] = self.angle
        return data

class BTNode_ActionAttack(bpy.types.Node, BehaviorTreeNodeBase):
    bl_idname = 'BTNode_ActionAttack'
    bl_label = "Attack"
    bl_icon = 'EVENT_A'

    def init(self, context):
        self.inputs.new('BehaviorTreeSocketType', "Parent")

class BTNode_Switch(bpy.types.Node, BehaviorTreeNodeBase):
    bl_idname = 'BTNode_Switch'
    bl_label = "Switch"
    bl_icon = 'LINENUMBERS_ON'

    switch_json: bpy.props.StringProperty(
        name="Switch JSON",
        description='スイッチ対象の変数名をJSONで記述\n例: {"variable": "state"}',
        default='{"variable": "state"}'
    ) # type: ignore

    case_count: bpy.props.IntProperty(
        name="Case Count",
        default=2,
        min=1,
        max=16,
        update=lambda self, ctx: self._rebuild_outputs()
    ) # type: ignore

    def _rebuild_outputs(self):
        # Switchノードもソケット数を増減させる際、既存のリンクを残す安全設計に修正 (改善点2)
        # Defaultソケットは常に末尾に1つ存在するため、Defaultソケットの手前でCaseソケットを制御します
        
        # 1. 既存のソケットをリスト化して退避（Defaultを除く）
        case_sockets = [s for s in self.outputs if s.name.startswith("Case_")]
        default_socket = next((s for s in self.outputs if s.name == "Default"), None)
        
        current_len = len(case_sockets)
        target_len = self.case_count

        if current_len < target_len:
            # 足りないCaseを、Defaultの「直前」に差し込むため一旦Defaultを削除して後から再追加
            if default_socket:
                self.outputs.remove(default_socket)
            for i in range(current_len, target_len):
                self.outputs.new('BehaviorTreeSocketType', f"Case_{i}")
            self.outputs.new('BehaviorTreeSocketType', "Default")
        elif current_len > target_len:
            # 不要なCaseを末尾（Defaultの直前）から削除
            for i in range(current_len - 1, target_len - 1, -1):
                self.outputs.remove(case_sockets[i])

    def init(self, context):
        self.inputs.new('BehaviorTreeSocketType', "Parent")
        for i in range(self.case_count):
            self.outputs.new('BehaviorTreeSocketType', f"Case_{i}")
        self.outputs.new('BehaviorTreeSocketType', "Default")

    def draw_buttons(self, context, layout):
        layout.label(text="Switch JSON:")
        layout.prop(self, "switch_json", text="")
        layout.prop(self, "case_count")
        try:
            import json
            json.loads(self.switch_json)
            layout.label(text="✓ Valid JSON", icon='CHECKMARK')
        except Exception:
            layout.label(text="⚠ Invalid JSON", icon='ERROR')

    def get_export_data(self):
        import json
        data = super().get_export_data()
        try:
            data["switch"] = json.loads(self.switch_json)
        except Exception:
            data["switch"] = self.switch_json
        data["case_count"] = self.case_count
        return data

class BTNodeCategory(NodeCategory):
    @classmethod
    def poll(cls, context):
        return context.space_data.tree_type == 'BehaviorTreeType'

node_categories = [
    BTNodeCategory('BT_CORE', "Core", items=[
        NodeItem('BTNode_Root'),
    ]),
    BTNodeCategory('BT_COMPOSITE', "Composite", items=[
        NodeItem('BTNode_Sequence'),
        NodeItem('BTNode_Selector'),
    ]),
    BTNodeCategory('BT_DECORATOR', "Decorator", items=[
        NodeItem('BTNode_Inverter'),
        NodeItem('BTNode_Branch'),
        NodeItem('BTNode_Switch'),
        NodeItem('BTNode_CondRandom'),
    ]),
    BTNodeCategory('BT_ACTION', "Action", items=[
        NodeItem('BTNode_ActionWait'),
        NodeItem('BTNode_ActionMoveTo'),
        NodeItem('BTNode_ActionTurn'),
        NodeItem('BTNode_ActionAttack'),
    ]),
]

classes = (
    BehaviorTreeSocket,
    BehaviorTreeNodeTree,
    CreateBehaviorTreeWorkspaceOperator,
    BTNode_Root,
    BTNode_Sequence,
    BTNode_Selector,
    BTNode_Inverter,
    BTNode_Branch,
    BTNode_CondRandom,
    BTNode_Switch,
    BTNode_ActionWait,
    BTNode_ActionMoveTo,
    BTNode_ActionTurn,
    BTNode_ActionAttack,
)

def register():
    for cls in classes:
        bpy.utils.register_class(cls)
        
    try:
        nodeitems_utils.register_node_categories("BEHAVIOR_TREE_CATEGORIES", node_categories)
    except Exception:
        nodeitems_utils.unregister_node_categories("BEHAVIOR_TREE_CATEGORIES")
        nodeitems_utils.register_node_categories("BEHAVIOR_TREE_CATEGORIES", node_categories)
        
    print("エネミー行動ツリーエディタが有効化されました。")


def unregister():
    nodeitems_utils.unregister_node_categories("BEHAVIOR_TREE_CATEGORIES")
    for cls in reversed(classes):
        bpy.utils.unregister_class(cls)
    print("エネミー行動ツリーエディタが無効化されました。")

if __name__ == "__main__":
    register()