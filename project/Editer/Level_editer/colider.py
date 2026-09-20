import bpy
import gpu
import gpu_extras.batch
import copy
import mathutils
# ==========================================
# 2. コライダー設定パネル
# ==========================================
class MYADDON_OT_add_collider(bpy.types.Operator):
    bl_idname = "myaddon.myaddon_ot_add_collider"
    bl_label = "Collider 追加"
    bl_description = "['collider'] カスタムプロパティを追加します"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        context.object["collider"] = "BOX"
        context.object["collider_center"]= mathutils.Vector((0,0,0))
        context.object["collider_size"]= mathutils.Vector((2,2,2))
        return {"FINISHED"}
    
class OBJECT_PT_collider(bpy.types.Panel):
    bl_idname = "OBJECT_PT_collider"
    bl_label = "Collider"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "object"

    @classmethod
    def poll(cls, context):
        # レール（CURVE）にはコライダー設定は不要
        obj = context.object
        return obj is not None and obj.type != 'CURVE'

    def draw(self, context):
        if "collider" in context.object:
            layout = self.layout
            layout.prop(context.object, '["collider"]', text="Type")
            layout.prop(context.object, '["collider_center"]', text="Collider Center")
            layout.prop(context.object, '["collider_size"]', text="Collider Size")
        else:
            self.layout.operator(MYADDON_OT_add_collider.bl_idname, text="Collider 追加")

# ==========================================
# 4. コライダー描画
# ==========================================
class DrawCollider:
    handle = None

    def draw_colider():
        vertices={"pos":[]}
        indices = []

        ofsets=[
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
            if not "collider" in object:
                continue

            center = mathutils.Vector((0,0,0))
            size = mathutils.Vector((2,2,2))

            center[0] = object["collider_center"][0]
            center[1] = object["collider_center"][1]
            center[2] = object["collider_center"][2]
            size[0] = object["collider_size"][0]
            size[1] = object["collider_size"][1]         
            size[2] = object["collider_size"][2]

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

        shader = gpu.shader.from_builtin("UNIFORM_COLOR")
        batch = gpu_extras.batch.batch_for_shader(shader,"LINES",vertices, indices=indices)

        color = [0.5, 1.0, 1.0, 1.0]
        shader.bind()
        shader.uniform_float("color", color)
        batch.draw(shader)