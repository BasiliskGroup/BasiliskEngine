import glm
import numpy as np
from ..render.image import Image

class Material():
    name: str = None
    """Name of the material"""  
    index: glm.int32
    """Index of the material in the material uniform"""  
    color: glm.vec3 = glm.vec3(1, 1, 1)
    """Color multiplier of the material"""   
    texture: Image = None
    """Key/name of the texture image in the image handler. If the image given does not exist, it must be loaded"""
    normal: Image = None
    """Key/name of the normal image in the image handler. If the image given does not exist, it must be loaded"""
    # PBR attributes  
    roughness: glm.float32
    """The PBR roughness of the material"""
    metallicness: glm.float32
    """The PBR metallicness of the material"""
    specular: glm.float32
    """The PBR specular value of the material"""

    def __init__(self, name: str, color: tuple=(255.0, 255.0, 255.0), texture: Image=None, normal: Image=None, roughness: float=0.5, metallicness: float=0.0, specular: float=0.5) -> None:
        """
        Basilisk Material object. Contains the data and images references used by the material.
        Args:
            name: str
                Identifier to be used by user
            color: tuple
                Base color of the material. Applies to textures as well
            texture: Basilisk Image
                The albedo map (color texture) of the material
            normal: Basilisk Image
                The normal map of the material.
            roughness & metallicness & specular: float
                PBR attributes of the material
        """
        
        self.index = glm.int32(0)
        
        self.name         = name
        self.color        = color
        self.texture      = texture
        self.normal       = normal
        self.roughness    = roughness
        self.metallicness = metallicness
        self.specular     = specular

    def __repr__(self) -> str:
        return f'<Basilisk Material | {self.name}, ({self.color.x}, {self.color.y}, {self.color.z}), {self.texture}>'

    @property
    def color(self):        return self._color
    @property
    def texture(self):      return self._texture
    @property
    def normal(self):       return self._normal
    @property
    def roughness(self):    return self._roughness
    @property
    def metallicness(self): return self._metallicness
    @property
    def specular(self):     return self._specular

    @color.setter
    def color(self, value: tuple | list | glm.vec3 | np.ndarray):
        if isinstance(value, tuple) or isinstance(value, list) or isinstance(value, np.ndarray):
            if len(value) != 3: raise ValueError(f"Material: Invalid number of values for color. Expected 3 values, got {len(value)} values")
            self._color = glm.vec3(value)
        elif isinstance(value, glm.vec3):
            self._color = glm.vec3(value)
        else:
            raise TypeError(f"Material: Invalid color value type {type(value)}")
        
    @texture.setter
    def texture(self, value: Image | None):
        if isinstance(value, Image) or isinstance(value, type(None)):
            self._texture = value
        else:
            raise TypeError(f"Material: Invalid texture value type {type(value)}")
        
    @normal.setter
    def normal(self, value: Image | None):
        if isinstance(value, Image) or isinstance(value, type(None)):
            self._normal = value
        else:
            raise TypeError(f"Material: Invalid normal value type {type(value)}")

    @roughness.setter
    def roughness(self, value: float | glm.float32):
        if isinstance(value, float):
            self._roughness = glm.float32(value)
        elif isinstance(value, glm.float32):
            self._roughness = glm.float32(value.value)
        else:
            raise TypeError(f"Material: Invalid roughness value type {type(value)}")

    @metallicness.setter
    def metallicness(self, value: float | glm.float32):
        if isinstance(value, float):
            self._metallicness = glm.float32(value)
        elif isinstance(value, glm.float32):
            self._metallicness = glm.float32(value.value)
        else:
            raise TypeError(f"Material: Invalid metallicness value type {type(value)}")
        
    @specular.setter
    def specular(self, value: float | glm.float32):
        if isinstance(value, float):
            self._specular = glm.float32(value)
        elif isinstance(value, glm.float32):
            self._specular = glm.float32(value.value)
        else:
            raise TypeError(f"Material: Invalid specular value type {type(value)}")