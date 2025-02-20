import bpy
import os
import shutil
import json

def get_texture_path(image):
    if image.packed_file:
        return None 
    
    if image.filepath:
        return bpy.path.abspath(image.filepath)
    return None

def process_lit_material(material, nodes, links, directory, texture_paths):
    material_info = {
        "diffuse": "",
        "normal": "",
        "orm": ""
    }

    texture_nodes = [node for node in nodes if node.type == 'TEX_IMAGE']

    for node in texture_nodes:
        if not node.image:
            continue
            
        image_name = os.path.basename(node.image.filepath) if node.image.filepath else node.image.name
        if not image_name.lower().endswith('.png'):
            image_name = f"{image_name}.png"

        for link in node.outputs[0].links:
            if 'Diffuse' in link.to_socket.name or 'Base Color' in link.to_socket.name:
                material_info['diffuse'] = image_name
            elif 'Normal' in link.to_socket.name:
                material_info['normal'] = image_name
            elif 'ORM' in link.to_socket.name or 'OcclusionRoughnessMetallic' in link.to_socket.name:
                material_info['orm'] = image_name
        
        # handle packed textures
        if node.image.packed_file:
            file_path = os.path.join(directory, image_name)
            node.image.filepath_raw = file_path
            node.image.file_format = 'PNG'
            node.image.save_render(file_path)
            texture_paths.add(file_path)
        
        # handle external textures
        else:
            src_path = get_texture_path(node.image)
            if src_path and os.path.exists(src_path):
                target_path = os.path.join(directory, image_name)
                texture_paths.add((src_path, target_path))

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
            color = node.outputs[0].default_value[:3]
            material_info["color"] = {
                "x": color[0],
                "y": color[1],
                "z": color[2]
            }
            break
    return material_info

def export_model(obj, directory):
    directory = os.path.join(directory, '')
    
    output_data = {
        "file": f"{obj.name}.fbx",
        "materials": []
    }

    texture_paths = set()

    for idx, slot in enumerate(obj.material_slots):
        mat = slot.material
        if mat and mat.use_nodes:
            shader_type = mat.get("shader", "lit")
            material_info = {
                "id": idx,
                "shader": shader_type
            }

            nodes = mat.node_tree.nodes
            links = mat.node_tree.links

            if shader_type == "unlit":
                material_info.update(process_unlit_material(mat, nodes))
            else:
                material_info.update(process_lit_material(mat, nodes, links, directory, texture_paths))

            output_data['materials'].append(material_info)

    json_filepath = os.path.join(directory, f"{obj.name}.model")
    with open(json_filepath, 'w') as f:
        json.dump(output_data, f, indent=4)

    fbx_filepath = os.path.join(directory, f"{obj.name}.fbx")
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj

    # fbx y-up export settings
    kwargs = {
        'filepath': fbx_filepath,
        'use_selection': True,
        'object_types': {'MESH'},
        'use_mesh_modifiers': True,
        'use_active_collection': False,
        'axis_forward': '-Z',  
        'axis_up': 'Y',        
        'global_scale': 1.0,          
        'apply_unit_scale': True,     
        'bake_space_transform': True,
        'apply_scale_options': 'FBX_SCALE_UNITS'
    }

    bpy.ops.export_scene.fbx(**kwargs)

    # Copy textures
    for texture_path in texture_paths:
        if isinstance(texture_path, tuple):
            # handle external textures
            src_path, target_path = texture_path
            if os.path.exists(src_path) and (not os.path.exists(target_path) or not os.path.samefile(src_path, target_path)):
                shutil.copy2(src_path, target_path)
        else:
            continue

    return output_data