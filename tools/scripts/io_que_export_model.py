import bpy
import os
import shutil
import json

def save_material_details(context, directory):
    directory = os.path.join(directory, '')
    
    obj = context.active_object

    if obj is not None and obj.type == 'MESH':
        print(f"Selected object: {obj.name}")
        
        output_data = {
            "file": f"{obj.name}.fbx",
            "materials": []
        }
        
        texture_paths = set()
        
        for idx, slot in enumerate(obj.material_slots):
            mat = slot.material
            if mat and mat.use_nodes:
                material_info = {
                    "id": idx,
                    "diffuse": "",
                    "normal": "",
                    "orm": ""
                }
                
                nodes = mat.node_tree.nodes
                links = mat.node_tree.links
                texture_nodes = [node for node in nodes if node.type == 'TEX_IMAGE']
                
                for node in texture_nodes:
                    image_name = node.image.name if node.image else 'None'
                    for link in node.outputs[0].links:  
                        if 'Diffuse' in link.to_socket.name or 'Base Color' in link.to_socket.name:
                            material_info['diffuse'] = image_name
                        elif 'Normal' in link.to_socket.name:
                            material_info['normal'] = image_name
                        elif 'ORM' in link.to_socket.name or 'OcclusionRoughnessMetallic' in link.to_socket.name:
                            material_info['orm'] = image_name
                    
                    if node.image and node.image.packed_file:
                        file_path = os.path.join(directory, image_name)
                        node.image.filepath_raw = file_path
                        node.image.file_format = 'PNG'
                        node.image.save_render(file_path)
                        print(f"Extracted embedded texture to {file_path}")
                        texture_paths.add(file_path)
                    
                    if node.image and node.image.filepath:
                        texture_paths.add(node.image.filepath)    

                output_data['materials'].append(material_info)
        
        json_filepath = os.path.join(directory, f"{obj.name}.model")
        with open(json_filepath, 'w') as f:
            json.dump(output_data, f, indent=4)
        print(f"Material details saved in JSON format to {json_filepath}")
        
        
        fbx_filepath = os.path.join(directory, f"{obj.name}.fbx")
        bpy.ops.export_scene.fbx(filepath=fbx_filepath, use_selection=True)
        print(f"Mesh exported to {fbx_filepath}")
        
        for texture_path in texture_paths:
            if os.path.exists(texture_path):
                target_path = os.path.join(directory, os.path.basename(texture_path))
                shutil.copy(texture_path, target_path)
                print(f"Copied texture {os.path.basename(texture_path)} to {target_path}")
            else:
                print(f"Texture file not found: {texture_path}")
        
    else:
        print("No mesh selected or active object is not a mesh.")

class SimpleOperator(bpy.types.Operator):
    """Save Material Details to File"""
    bl_idname = "object.save_material_details"
    bl_label = "Save Material Details"
    
    directory: bpy.props.StringProperty(
        subtype='DIR_PATH'
    )
    
    def execute(self, context):
        save_material_details(context, self.directory)
        return {'FINISHED'}
    
    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}

def register():
    bpy.utils.register_class(SimpleOperator)

def unregister():
    bpy.utils.unregister_class(SimpleOperator)

if __name__ == "__main__":
    register()
    
    
    # bpy.ops.object.save_material_details()

    
    # Uncomment the line below to unregister when needed
    # unregister()
