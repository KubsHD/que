import bpy
import os
from bpy.props import StringProperty
from . import io_que_model_lib as model_lib

class QUE_PT_export_model_panel(bpy.types.Panel):
    bl_label = "QUE Model Exporter"
    bl_idname = "QUE_PT_export_model_panel"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'QUE Export'

    def draw(self, context):
        layout = self.layout
        scene = context.scene

        layout.prop(scene, "que_model_export_path", text="Export Path")
        
        box = layout.box()
        selected_meshes = [obj for obj in context.selected_objects if obj.type == 'MESH']
        if selected_meshes:
            box.label(text=f"Selected meshes: {len(selected_meshes)}")
            for obj in selected_meshes:
                box.label(text=f"- {obj.name}")
        else:
            box.label(text="No meshes selected!", icon='ERROR')
        
        row = layout.row()
        row.scale_y = 2.0
        row.operator("que.export_model", text="EXPORT MODEL", icon='EXPORT')

class QUE_OT_export_model(bpy.types.Operator):
    bl_idname = "que.export_model"
    bl_label = "Export QUE Model"
    bl_description = "Export selected mesh as QUE model"
    
    def execute(self, context):
        export_path = bpy.path.abspath(context.scene.que_model_export_path)
        
        if not export_path:
            self.report({'ERROR'}, "Export path not set!")
            return {'CANCELLED'}
        
        selected_meshes = [obj for obj in context.selected_objects if obj.type == 'MESH']
        
        if not selected_meshes:
            self.report({'ERROR'}, "No meshes selected!")
            return {'CANCELLED'}
        
        if len(selected_meshes) > 1:
            self.report({'WARNING'}, "Multiple meshes selected. Exporting all selected meshes.")
        
        try:
            os.makedirs(os.path.dirname(export_path), exist_ok=True)
        except PermissionError:
            self.report({'ERROR'}, f"Permission denied creating directory: {os.path.dirname(export_path)}")
            return {'CANCELLED'}
        except Exception as e:
            self.report({'ERROR'}, f"Failed to create directory: {str(e)}")
            return {'CANCELLED'}
        
        try:
            # Export each selected mesh
            for obj in selected_meshes:
                model_lib.export_model(obj, os.path.dirname(export_path))
                self.report({'INFO'}, f"Successfully exported model: {obj.name}")
            
            return {'FINISHED'}
            
        except Exception as e:
            self.report({'ERROR'}, f"Export failed: {str(e)}")
            return {'CANCELLED'}

# blender boilerplate
def register():
    if hasattr(bpy.types.Scene, "que_model_export_path"):
        del bpy.types.Scene.que_model_export_path
        
    bpy.types.Scene.que_model_export_path = StringProperty(
        name="Export Path",
        description="Path to export the model file",
        default="",
        subtype='FILE_PATH',
        maxlen=1024
    )
    
    bpy.utils.register_class(QUE_PT_export_model_panel)
    bpy.utils.register_class(QUE_OT_export_model)

def unregister():
    if hasattr(bpy.types.Scene, "que_model_export_path"):
        del bpy.types.Scene.que_model_export_path
    
    bpy.utils.unregister_class(QUE_PT_export_model_panel)
    bpy.utils.unregister_class(QUE_OT_export_model)

if __name__ == "__main__":
    register()
