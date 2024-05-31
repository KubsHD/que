import bpy
import os
import shutil
import json

def process_lit_material(material, nodes, links, directory, texture_paths):
    material_info = {
        "diffuse": "",
        "normal": "",
        "orm": ""
    }

    texture_nodes = [node for node in nodes if node.type == 'TEX_IMAGE']

    for node in texture_nodes:
        image_name = node.image.name if node.image else 'None'
        for link in node.outputs[0].links:  # assuming the first output is used, this may need adjustment
            if 'Diffuse' in link.to_socket.name or 'Base Color' in link.to_socket.name:
                material_info['diffuse'] = image_name
            elif 'Normal' in link.to_socket.name:
                material_info['normal'] = image_name
            elif 'ORM' in link.to_socket.name or 'OcclusionRoughnessMetallic' in link.to_socket.name:
                material_info['orm'] = image_name
        
        # Save packed textures
        if node.image and node.image.packed_file:
            file_path = os.path.join(directory, image_name)
            node.image.filepath_raw = file_path
            node.image.file_format = 'PNG'
            node.image.save_render(file_path)
            print(f"Extracted embedded texture to {file_path}")
            texture_paths.add(file_path)
        
        if node.image and node.image.filepath:
            texture_paths.add(node.image.filepath)    

    return material_info

def process_unlit_material(material, nodes):
    material_info = {
        "color": {
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
        }
    }
    for node in nodes:
        if node.type == 'RGB':
            color = node.outputs[0].default_value[:3]  # Get the RGB values
            material_info["color"] = {
                "x": color[0],
                "y": color[1],
                "z": color[2]
            }
            break
    return material_info

def save_material_details(context, directory):
    # Ensure the directory ends with a separator
    directory = os.path.join(directory, '')
    
    # Get the currently selected object
    obj = context.active_object

    if obj is not None and obj.type == 'MESH':
        print(f"Selected object: {obj.name}")
        
        output_data = {
            "file": f"{obj.name}.fbx",
            "materials": []
        }
        
        texture_paths = set()
        
        # Iterate over each material slot in the object
        for idx, slot in enumerate(obj.material_slots):
            mat = slot.material
            if mat and mat.use_nodes:
                shader_type = mat.get("shader", "lit")  # Default to "lit" if the custom property is not set
                material_info = {
                    "id": idx,
                    "shader": shader_type
                }
                
                nodes = mat.node_tree.nodes
                links = mat.node_tree.links
                
                if shader_type == "unlit":
                    # Process unlit material
                    material_info.update(process_unlit_material(mat, nodes))
                else:  # Process lit material
                    material_info.update(process_lit_material(mat, nodes, links, directory, texture_paths))
                
                output_data['materials'].append(material_info)
        
        # Save the details to a JSON file
        json_filepath = os.path.join(directory, f"{obj.name}.model")
        with open(json_filepath, 'w') as f:
            json.dump(output_data, f, indent=4)
        print(f"Material details saved in JSON format to {json_filepath}")
        
        fbx_filepath = os.path.join(directory, f"{obj.name}.fbx")
        bpy.ops.export_scene.fbx(filepath=fbx_filepath, use_selection=True)
        print(f"Mesh exported to {fbx_filepath}")
        
        # Copy texture files to the same directory
        for texture_path in texture_paths:
            target_path = os.path.join(directory, os.path.basename(texture_path))
            if not os.path.samefile(texture_path, target_path):
                shutil.copy(texture_path, target_path)
                print(f"Copied texture {os.path.basename(texture_path)} to {target_path}")
            else:
                print(f"Skipped copying texture {os.path.basename(texture_path)} as source and destination are the same")
        
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
    
    # Uncomment the line below to call the operator manually
    # bpy.ops.object.save_material_details()

    # Uncomment the line below to unregister when needed
    # unregister()
