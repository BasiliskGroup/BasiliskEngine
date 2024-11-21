import os
from scripts.render.vbo_handler import ModelVBO

def preload(scene, model_directory_path: str=None, image_directory_path: str=None):

    vbos = scene.vao_handler.vbo_handler.vbos

    # Load all models
    if model_directory_path:
        model_directory = os.fsencode(model_directory_path)
        for file in os.listdir(model_directory):
            filename = os.fsdecode(file)
            if filename.endswith(".obj"): 
                vbos[filename[:-4]] = ModelVBO(scene.ctx, model_directory_path + "/" + filename)

    # Load all models
    if image_directory_path:
        image_directory = os.fsencode(image_directory_path)
        for file in os.listdir(image_directory):
            filename = os.fsdecode(file)
            if filename.endswith(".png"):
                scene.project.texture_handler.load_texture(image_directory_path, '/' + filename)