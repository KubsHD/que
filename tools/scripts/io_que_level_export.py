import bpy
import os
import json

def save_level_details(context, directory):
    # Ensure the directory ends with a separator
    directory = os.path.join(directory, '')

    # Get the current Blender file name
    blend_file_name = os.path.splitext(bpy.path.basename(bpy.data.filepath))[0]
    level_file_name = f"{blend_file_name}.level"

    level_data = {
        "name": "LEVEL",
        "objects": []
    }

    # Iterate over the selected objects
    selected_objects = context.selected_objects
    for obj in selected_objects:
        if obj.type == 'MESH':
            print(f"Processing object: {obj.name}")

            # Gather the object's transformation data
            obj_data = {
                "model": f"{obj.name}.model",
                "position": [obj.location.x, obj.location.y, obj.location.z],
                "rotation": [obj.rotation_euler.x, obj.rotation_euler.y, obj.rotation_euler.z],
                "scale": [obj.scale.x, obj.scale.y, obj.scale.z]
            }

            level_data["objects"].append(obj_data)

    # Save the level data to a JSON file
    level_file_path = os.path.join(directory, level_file_name)
    with open(level_file_path, 'w') as f:
        json.dump(level_data, f, indent=4)
    print(f"Level details saved in JSON format to {level_file_path}")

class SaveLevelOperator(bpy.types.Operator):
    """Save Level Details to File"""
    bl_idname = "object.save_level_details"
    bl_label = "Save Level Details"
    
    directory: bpy.props.StringProperty(
        subtype='DIR_PATH'
    )
    
    def execute(self, context):
        save_level_details(context, self.directory)
        return {'FINISHED'}
    
    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}

def register():
    bpy.utils.register_class(SaveLevelOperator)

def unregister():
    bpy.utils.unregister_class(SaveLevelOperator)

if __name__ == "__main__":
    register()
    
    # Uncomment the line below to call the operator manually
    # bpy.ops.object.save_level_details()

    # Uncomment the line below to unregister when needed
    # unregister()
