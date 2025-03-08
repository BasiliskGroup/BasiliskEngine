import numpy as np
import glm
from ..render.image import Image


def validate_int(module: str, name: str, value: int | float | glm.int32) -> float:
    if isinstance(value, int) or isinstance(value, float):
        return int(round(value))
    elif isinstance(value, glm.int32):
        return int(value.value)
    else:
        raise TypeError(f"{module}: Invalid {name} value type {type(value)}. Expected float.")

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
    elif isinstance(value, glm.vec3) or isinstance(value, int) or isinstance(value, float):
        return glm.vec3(value)
    else:
        raise TypeError(f"{module}: Invalid {name} value type {type(value)}. Expected glm.vec3")

def validate_tuple3(module: str, name: str, value: tuple | list | glm.vec3 | np.ndarray) -> tuple:
    if isinstance(value, tuple) or isinstance(value, list) or isinstance(value, np.ndarray):
        if len(value) != 3: raise ValueError(f"{module}: Invalid number of values for {name}. Expected 3 values, got {len(value)} values")
        return tuple(value)
    elif isinstance(value, glm.vec3):
        return (value.x, value.y, value.z)
    if isinstance(value, int) or isinstance(value, float):
        return (value, value, value)
    else:
        raise TypeError(f"{module}: Invalid {name} value type {type(value)}. Expected tuple of size 3")

def validate_image(module: str, name: str, value: Image | None) -> Image | None:
    """Accepts none as a value for no image"""
    if isinstance(value, Image) or isinstance(value, type(None)):
        return value
    else:
        raise TypeError(f"{module}: Invalid {name} value type {type(value)}. Expected bsk.Image or None")
    
def validate_color(module: str, name: str, value: tuple | list | np.ndarray | int | float | None) -> tuple:
    if isinstance(value, tuple) or isinstance(value, list) or isinstance(value, np.ndarray):
        if len(value) == 4:
            return tuple(map(lambda x: x / 255, value))
        elif len(value) == 3:
            return (*tuple(map(lambda x: x / 255, value)), 1.0)
        else:
            raise TypeError(f"{module}: Invalid number of values for {name}. Expected 3 or 4 values, got {len(value)} values")
    elif isinstance(value, int) or isinstance(value, float):
        v = value / 255
        return (v, v, v, 1.0)
    else:
        raise TypeError(f"{module}: Invalid {name} value type {type(value)}. Expected tuple of size 3 or 4")

def validate_rect(rect) -> tuple:
    if not (isinstance(rect, tuple) or isinstance(rect, list) or isinstance(rect, np.ndarray)):
        raise TypeError(f'Invalid rect type: {type(rect)}. Expected a tuple, list, or numpy array')
    if len(rect) != 4:
        raise TypeError(f'Invalid number of rect values. Expected 4 values, got {len(rect)}')
    return list(rect)

def validate_point(point) -> tuple:
    if not (isinstance(point, tuple) or isinstance(point, list) or isinstance(point, np.ndarray)):
        raise TypeError(f'Invalid rect type: {type(point)}. Expected a tuple, list, or numpy array')
    if len(point) != 2:
        raise TypeError(f'Invalid number of rect values. Expected 2 values, got {len(point)}')
    return list(point)