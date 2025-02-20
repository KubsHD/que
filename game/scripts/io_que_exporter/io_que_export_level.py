import bpy
import json
import os
from mathutils import Matrix, Euler
from math import degrees, radians
from bpy.props import StringProperty
from bpy_extras.io_utils import ExportHelper
from . import io_que_model_lib as model_lib

def vector_to_object(vector, decimals=4):
    return {
        "x": round(vector[0], decimals),
        "y": round(vector[1], decimals),
        "z": round(vector[2], decimals)
    }

def is_object_visible(obj):
    return (not obj.hide_viewport and  
            not obj.hide_get() and      
            not obj.hide_render)        

def export_level(context, export_path):
    level_data = {
        "name": os.path.splitext(os.path.basename(export_path))[0],
        "objects": [],
        "entities": []
    }
    
    for obj in context.scene.objects:
        if obj.type == 'MESH' and is_object_visible(obj):
            pos = vector_to_object([
                obj.location.x,     
                obj.location.z,    
                -obj.location.y     
            ])
            
            rot = obj.rotation_euler
            rot_degrees = vector_to_object([
                degrees(rot.x),     
                degrees(rot.z),     
                -degrees(rot.y)     
            ])
            
            scale = vector_to_object([
                obj.scale.x,         
                obj.scale.z,         
                obj.scale.y          
            ])
            
            # get the col property, default to true if not found
            col = bool(obj.get("col", False))
            
            object_data = {
                "model": f"{obj.name}.model",
                "position": pos,
                "rotation": rot_degrees,
                "scale": scale,
                "col": col
            }
            level_data["objects"].append(object_data)
            
            model_lib.export_model(obj, os.path.dirname(export_path))
    
    for obj in context.scene.objects:
        if obj.type == 'EMPTY' and is_object_visible(obj):
            pos = vector_to_object([
                obj.location.x,      
                obj.location.z,      
                -obj.location.y      
            ])
            entity_data = {
                "name": obj.name,
                "position": pos
            }
            level_data["entities"].append(entity_data)
    
    with open(export_path, 'w') as f:
        json.dump(level_data, f, indent=4)

class QUE_PT_export_panel(bpy.types.Panel):
    bl_label = "QUE Level Exporter"
    bl_idname = "QUE_PT_export_panel"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'QUE Export'

    def draw(self, context):
        layout = self.layout
        scene = context.scene

        layout.prop(scene, "que_export_path", text="Export Path")
        
        row = layout.row()
        row.scale_y = 2.0  
        row.operator("que.export_level", text="EXPORT LEVEL", icon='EXPORT')

class QUE_OT_export_level(bpy.types.Operator):
    bl_idname = "que.export_level"
    bl_label = "Export QUE Level"
    bl_description = "Export level data and models"
    
    def execute(self, context):
        export_path = bpy.path.abspath(context.scene.que_export_path)
        
        if not export_path:
            self.report({'ERROR'}, "Export path not set!")
            return {'CANCELLED'}
            
        # create directory if it doesn't exist
        try:
            os.makedirs(os.path.dirname(export_path), exist_ok=True)
        except PermissionError:
            self.report({'ERROR'}, f"Permission denied creating directory: {os.path.dirname(export_path)}")
            return {'CANCELLED'}
        except Exception as e:
            self.report({'ERROR'}, f"Failed to create directory: {str(e)}")
            return {'CANCELLED'}
        
        try:
            export_level(context, export_path)
            self.report({'INFO'}, f"Successfully exported level to {export_path}")
            return {'FINISHED'}
        except Exception as e:
            self.report({'ERROR'}, f"Export failed: {str(e)}")
            return {'CANCELLED'}


# blender boilerplate
def register():
    if hasattr(bpy.types.Scene, "que_export_path"):
        del bpy.types.Scene.que_export_path
        
    bpy.types.Scene.que_export_path = StringProperty(
        name="Export Path",
        description="Path to export the level file",
        default="",
        subtype='FILE_PATH',
        maxlen=1024
    )
    
    bpy.utils.register_class(QUE_PT_export_panel)
    bpy.utils.register_class(QUE_OT_export_level)

def unregister():
    if hasattr(bpy.types.Scene, "que_export_path"):
        del bpy.types.Scene.que_export_path
    
    bpy.utils.unregister_class(QUE_PT_export_panel)
    bpy.utils.unregister_class(QUE_OT_export_level)

if __name__ == "__main__":
    register()
