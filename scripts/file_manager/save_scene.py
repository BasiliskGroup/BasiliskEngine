import os
import json


def save_scene(scene, local_file_name=None, abs_file_path=None):
    scene_data = {}

    scene_data["asset"] = {"version": "2.0"}
    scene_data["scene"] = 0
    scene_data["buffers"] = []
    scene_data["meshes"] = []
    scene_data["images"] = []
    scene_data["textures"] = []
    scene_data["materials"] = []
    scene_data["nodes"] = []
    scene_data["scenes"] = [{}]

    buffer_indices = save_buffers(scene, scene_data)
    image_indices, texture_indices = save_images(scene, scene_data)
    mtl_indices = save_materials(scene, scene_data, texture_indices)
    save_nodes(scene, scene_data, mtl_indices, buffer_indices)

    if local_file_name:
        with open(f'saves\{local_file_name}.gltf', 'w') as file:
            json.dump(scene_data, file, ensure_ascii=False, indent=4)
    else:
        with open(abs_file_path, 'w') as file:
            json.dump(scene_data, file, ensure_ascii=False, indent=4)

def save_buffers(scene, scene_data):
    buffer_indices = {}
    buffer_file_path = "models"

    for i, file in enumerate(os.listdir(buffer_file_path)):
        scene_data["buffers"].append({})
        scene_data["buffers"][-1]["uri"] = file
        buffer_indices[file[:-4]] = i

    return buffer_indices

def save_images(scene, scene_data):
    image_indices = {}
    texture_indices = {}
    image_file_path = "textures"
    
    for i, file in enumerate(os.listdir(image_file_path)):
        scene_data["images"].append({})
        scene_data["textures"].append({})
        scene_data["images"][-1]["uri"] = file
        scene_data["textures"][-1]["sampler"] = i
        image_indices[file[:-4]] = i
        texture_indices[file[:-4]] = i

    return image_indices, texture_indices

def save_materials(scene, scene_data, texture_indices):
    mtl_indices = {}

    # Add the base mtl
    scene_data["materials"].append({})
    scene_data["materials"][-1]["name"] = "base"
    scene_data["materials"][-1]["pbrMetallicRoughness"] = {
        "baseColorFactor": [0.8, 0.8, 0.8, 1.0],
        "metallicFactor" : .5,
        "roughnessFactor" : 64
    }
    mtl_indices['base'] = 0

    materials = scene.material_handler.materials
    for i, mtl_key in enumerate(materials):
        mtl = materials[mtl_key]
        scene_data["materials"].append({})

        scene_data["materials"][-1]["name"] = mtl_key
        scene_data["materials"][-1]["pbrMetallicRoughness"] = {
            "baseColorFactor": [mtl.color.x, mtl.color.y, mtl.color.z, mtl.alpha.value],
            "metallicFactor" : mtl.specular.value,
            "roughnessFactor" : mtl.specular_exponent.value
        }

        if mtl.texture:
            scene_data["materials"][-1]["pbrMetallicRoughness"]["baseColorTexture"] = {
                "index": texture_indices[mtl.texture],
                "texCoord": 1
            }
        
        if mtl.normal_map:
            scene_data["materials"][-1]["normalTexture"] = {
                "index": texture_indices[mtl.normal_map],
                "texCoord": 1,
                "scale" : 1
            }

        mtl_indices[mtl_key] = i + 1
    
    return mtl_indices


def save_nodes(scene, scene_data, mtl_indices, buffer_indices):
    mtl_names = list(scene.material_handler.material_ids.keys())
    for node in scene.node_handler.nodes:
        scene_data["nodes"].append({})
        scene_data["nodes"][-1]["name"] = node.name

        scene_data["nodes"][-1]["translation"] = node.position.x, node.position.y, node.position.z
        scene_data["nodes"][-1]["scale"]       = node.scale.x, node.scale.y, node.scale.z
        scene_data["nodes"][-1]["rotation"]    = node.rotation.x, node.rotation.y, node.rotation.z

        if node.model.vbo == "cube":
            scene_data["nodes"][-1]["mesh"] = "cube"
        elif node.model.vbo:
            scene_data["nodes"][-1]["mesh"] = buffer_indices[node.model.vbo]
        
        if node.model.material !=  None:
            scene_data["nodes"][-1]["material"] = mtl_indices[mtl_names[node.model.material]]