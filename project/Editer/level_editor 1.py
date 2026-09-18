import bpy
import math
import bpy_extras
import gpu
import gpu_extras.batch
import copy
import mathutils
bl_info={
    "name": "レベルエディタ",
    "author": "Fuku Akira",
    "version": (1,0),
    "blender": (3,3,1),
    "location": "",
    "description": "レベルエディタ",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Object"
}

#オペレーター　カスタムプロパティ['collider7]追加
class MYADDON_OT_add_collider(bpy.types.Operator):
    bl_idname = "myaddon.myaddon_ot_add_collider"
    bl_label = "Collider 追加"
    bl_description = "['collider'] カスタムプロパティを追加します"
    bl_options = {"REGISTER", "UNDO"}
    def execute(self, context):
        #['collider'] カスタムプロパティを追加
        context.object["collider"] = "BOX"
        context.object["collider_center"]= mathutils.Vector((0,0,0))
        context.object["collider_size"]= mathutils.Vector((2,2,2))
        return {"FINISHED"}
    
class OBJECT_PT_collider(bpy.types.Panel):
    """オブジェクトのコライダーパネル"""
    bl_idname = "OBJECT_PT_collider"
    bl_label = "Collider"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "object"

    def draw(self, context):
        if  "collider" in context.object:
            layout = self.layout
            layout.prop(context.object, '["collider"]', text="Type")
            layout.prop(context.object, '["collider_center"]', text="Collider Center")
            layout.prop(context.object, '["collider_size"]', text="Collider Size")
        else:
            #プロパティがなければ、プロパティ追加ボタンを表示
            self.layout.operator(MYADDON_OT_add_collider.bl_idname,text="Collider 追加")

#コライダー描画
class DrawCollider:
    handle =None

    def draw_colider():
        vertices={"pos":[]}
        indices = []

        ofsets=[
                 [-0.5, -0.5, -0.5],#左下後
                 [+0.5, -0.5, -0.5],#右下後
                 [-0.5, +0.5, -0.5],#左上後
                 [+0.5, +0.5, -0.5],#右上後
                 [-0.5, -0.5, +0.5],#左下前
                 [+0.5, -0.5, +0.5],#右下前
                 [-0.5, +0.5, +0.5],#左上前
                 [+0.5, +0.5, +0.5],#右上前
                
        ]
        size =[2,2,2]

        for object in bpy.context.scene.objects:
            if not "collider"  in object:
                continue

            center =mathutils.Vector((0,0,0))
            size =mathutils.Vector((2,2,2))

            center[0] =object["collider_center"][0]
            center[1] =object["collider_center"][1]
            center[2] =object["collider_center"][2]
            size[0] =object["collider_size"][0]
            size[1] =object["collider_size"][1]         
            size[2] =object["collider_size"][2]


            start=len(vertices["pos"])
            for ofset in ofsets:
                pos =copy.copy(center)
                pos.x += ofset[0] * size[0]
                pos.y += ofset[1] * size[1]
                pos.z += ofset[2] * size[2]

                pos =object.matrix_world @ pos
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

        shader =gpu.shader.from_builtin("UNIFORM_COLOR")

        batch =gpu_extras.batch.batch_for_shader(shader,"LINES",vertices, indices=indices)

        color =[0.5,1.0,1.0,1.0]
        shader.bind()
        shader.uniform_float("color",color)
        batch.draw(shader)


#パネル　ファイル名
class OBJECT_PT_file_name(bpy.types.Panel):
    """オブジェクトのファイルネームパネル"""
    bl_idname = "OBJECT_PT_file_name"
    bl_label = "FileName"
    bl_space_type = "PROPERTIES"
    bl_region_type= "WINDOW"
    bl_context = "object"
    #サブメニューの描画
    def draw(self, context):
       
     self.layout.operator(MYADDON_OT_stretch_vertex.bl_idname)
     self.layout.operator(MYADDON_OT_ICO_sphere.bl_idname,text=MYADDON_OT_ICO_sphere.bl_label)
     self.layout.operator(MYADDON_OT_export_scene.bl_idname,text=MYADDON_OT_export_scene.bl_label)
     

     if "file_name" in context.object:
         # 既にプロパティがあれば、プロパティを表示
         self.layout.prop(context.object, '["file_name"]', text=self.bl_label)
     else:
         #プロパティがなければ、プロパティ追加ボタンを表示
         self.layout.operator(MYADDON_OT_add_filwname.bl_idname,text=MYADDON_OT_add_filwname.bl_label)
    
  


#カスタムプロパティ
class MYADDON_OT_add_filwname(bpy.types.Operator):
    bl_idname = "myaddon.myaddon_ot_add_filename"
    bl_label = "FileName 追加"
    bl_description = "['file_name'] カスタムプロパティを追加します"
    bl_options = {"REGISTER", "UNDO"}
    def execute(self, context):
        #['file_name'] カスタムプロパティを追加
        context.object["file_name"] = ""
        return {"FINISHED"}

# オペレータ 頂点を伸ばす
class MYADDON_OT_stretch_vertex(bpy.types. Operator):
    bl_idname = "myaddon.myaddon_ot_stretch_vertex"
    bl_label = "頂点を伸ばす"
    bl_description = "頂点座標を引っ張って伸ばします"
    #リドゥ、アンドゥ可能オプション
    bl_options = {'REGISTER', 'UNDO'}
    #メニューを実行したときに呼ばれるコールバック関数
    def execute(self, context):
        bpy.data.objects["Cube"].data.vertices[0].co.x += 1.0 
        print("頂点を伸ばしました。")
        #オペレータの命令終了を通知
        return {'FINISHED'}   
# オペレータ ICO球生成
class MYADDON_OT_ICO_sphere(bpy.types. Operator):
    bl_idname = "myaddon.myaddon_ot_create_object"
    bl_label = "ICO球生成"
    bl_description = "ICO球を生成します"
    #リドゥ、アンドゥ可能オプション
    bl_options = {'REGISTER', 'UNDO'}
    #メニューを実行したときに呼ばれるコールバック関数
    def execute(self, context):
        bpy.ops.mesh.primitive_ico_sphere_add()
        print("ICO球を生成しました")
        #オペレータの命令終了を通知
        return {'FINISHED'}   
# オペレータ シーン出力
class MYADDON_OT_export_scene(bpy.types. Operator, bpy_extras.io_utils.ExportHelper):
    bl_idname = "myaddon.myaddon_ot_export_scene"
    bl_label = "シーン出力"
    bl_description = "シーン情報をExportする"
    filename_ext = ".scene"
    #メニューを実行したときに呼ばれるコールバック関数
    def execute(self, context):
        print("シーン情報をExportします")
        self.export()
        print("シーン情報をExportしました")
        self.report({'INFO'}, "シーン情報をExportしました")
        #オペレータの命令終了を通知
        return {'FINISHED'}   
    def export(self):
        """シーン情報をExportする処理"""
        print("シーン情報出力開始...%r" % self.filepath)
        with open(self.filepath, 'wt') as file:
            self.Write_and_print(file, "SCENE")
            for object in bpy.context.scene.objects:
                if(object.parent):
                    continue
                self.parse_scene_recursive(file, object, 0)
                if object.parent is not None:
                    self.Write_and_print(file, "parent: " + object.parent.name)
                self.Write_and_print(file, "")
    def Write_and_print(self, file, text):
        """ファイルにテキストを書き込み、同時にコンソールにも出力する"""
        print(text)
        file.write(text + "\n")
    def parse_scene_recursive(self, file, object, level):
        """シーン解析用再帰関数"""
        #インデントを作成
        indent =''
        for i in range(level):
            indent += "\t"
        #オブジェクトの情報を出力
        self.Write_and_print(file, indent + object.type )
        trans, rot, scale =object.matrix_local.decompose()
        rot = rot.to_euler()
        rot.x=math.degrees(rot.x)
        rot.y=math.degrees(rot.y)
        rot.z=math.degrees(rot.z)

        self.Write_and_print(file, indent + "Trans(%f, %f, %f)" % (trans.x, trans.y, trans.z))
        self.Write_and_print(file, indent + "Rot(%f, %f, %f)" % (rot.x, rot.y, rot.z))
        self.Write_and_print(file, indent + "Scale(%f, %f, %f)" % (scale.x, scale.y, scale.z))
        self.Write_and_print(file, "" )
        #カスタムプロパティ
        if "file_name" in object:
            self.Write_and_print(file, indent + "Name %s" % object["file_name"])
        if "collider" in object:
             self.Write_and_print(file, indent + "Collider %s" % object["collider"])
             temp_str= indent +"CC(%f,%f,%f)"
             temp_str =temp_str % (object["collider_center"][0], object["collider_center"][1], object["collider_center"][2])
             self.Write_and_print(file, temp_str)
             temp_str= indent +"CS(%f,%f,%f)"
             temp_str =temp_str % (object["collider_size"][0], object["collider_size"][1], object["collider_size"][2])
             self.Write_and_print(file, temp_str)
        self.Write_and_print(file, indent + 'End')
        self.Write_and_print(file,'')
        #子オブジェクトがある場合は再帰的に解析
        for child in object.children:
            self.parse_scene_recursive(file, child, level + 1)


    #トップバーの拡張メニュー
class TOPBAR_MT_my_menu(bpy.types. Menu):
    #Blenderがクラスを識別する為の固有の文字列
    bl_idname = "TOPBAR_MT_my_menu"
    #メニューのラベルとして表示される文字列
    bl_label = "MyMenu"
    #著者表示用の文字列
    bl_description = "拡張メニュー by " + bl_info["author"]
    # サブメニューの描画
    def draw(self, context):
    #トップバーの「エディターメニュー」に項目(オペレータ)を追加 
         self.layout.operator(MYADDON_OT_stretch_vertex.bl_idname, text=MYADDON_OT_stretch_vertex.bl_label)
         self.layout.operator(MYADDON_OT_ICO_sphere.bl_idname, text=MYADDON_OT_ICO_sphere.bl_label)
         self.layout.operator(MYADDON_OT_export_scene.bl_idname, text=MYADDON_OT_export_scene.bl_label)
         
        #既存のメニューにサブメニューを追加
    def submenu(self, context):
    # ID指定でサブメニューを追加
            self.layout.menu(TOPBAR_MT_my_menu.bl_idname)


classes=(
     TOPBAR_MT_my_menu,
     MYADDON_OT_export_scene,
     MYADDON_OT_ICO_sphere,
     MYADDON_OT_stretch_vertex,
    
     MYADDON_OT_add_filwname,
     OBJECT_PT_file_name,
     MYADDON_OT_add_collider,
     OBJECT_PT_collider,
     )
#class MYADDON_OT_stretch_vertex(bpy.types.Operator):

     
#メニュー項目
#def draw_my_menu_to_topbar(self, context):
 #   self.layout.menu("TOPBAR_MT_my_menu")

def register():
    for cls in classes:
         bpy.utils.register_class(cls)
    bpy.types.TOPBAR_MT_editor_menus.append(TOPBAR_MT_my_menu.submenu)
    DrawCollider.handle=bpy.types.SpaceView3D.draw_handler_add(DrawCollider.draw_colider,(),"WINDOW","POST_VIEW")
    print("レベルエディタが有効化されました。")

def unregister():
    bpy.types.SpaceView3D.draw_handler_remove(DrawCollider.handle,"WINDOW")
    bpy.types.TOPBAR_MT_editor_menus.remove(TOPBAR_MT_my_menu.submenu)
   
    for cls in classes:
         bpy.utils.unregister_class(cls)
    print("レベルエディタが無効化されました。")





if __name__ == "__main__":
    register()

