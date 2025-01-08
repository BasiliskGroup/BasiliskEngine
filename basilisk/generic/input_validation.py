import numpy as np
import glm
from ..render.image import Image


def validate_float(module: str, name: str, value: float | int | glm.float32) -> float:
    if isinstance(value, float) or isinstance(value, int):
        return float(value)
    elif isinstance(value, glm.float32):
        return float(value.value)
    else:
        raise TypeError(f"{module}: Invalid {name} value type {type(value)}. Expected float.")
    
def validate_glm_vec3(module: str, name: str, value: tuple | list | glm.vec3 | np.ndarray) -> glm.vec3:
    if isinstance(value, tuple) or isinstance(value, list) or isinstance(value, np.ndarray):
        if len(value) != 3: raise ValueError(f"{module}: Invalid number of values for {name}. Expected 3 values, got {len(value)} values")
        return glm.vec3(value)
    elif isinstance(value, glm.vec3):
        return glm.vec3(value)
    else:
        raise TypeError(f"{module}: Invalid {name} value type {type(value)}. Expected glm.vec3")

def validate_image(module: str, name: str, value: Image | None) -> Image | None:
    "Accepts none as a value for no image"
    if isinstance(value, Image) or isinstance(value, type(None)):
        return value
    else:
        raise TypeError(f"{module}: Invalid {name} value type {type(value)}. Expected bsk.Image or None")