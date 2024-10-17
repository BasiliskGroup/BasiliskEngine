import shutil


def drag_file(file: str) -> str:
    if file.endswith(".obj"):
        return shutil.copy(file, "models")
    elif file.endswith(".png") or file.endswith(".jpg"):
        return shutil.copy(file, "textures")