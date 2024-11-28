import glm
import numpy as np
from ..render.image import Image

class Material():
    name: str = None
    """Name of the material"""  
    index: 0
    """Index of the material in the material uniform"""  
    color: glm.vec3 = glm.vec3(1, 1, 1)
    """Color multiplier of the material"""   
    texture: Image = None
    """Key/name of the texture image in the image handler. If the image given does not exist, it must be loaded"""
    normal: Image = None
    """Key/name of the normal image in the image handler. If the image given does not exist, it must be loaded"""
    # PBR attributes  
    roughness: float
    """The PBR roughness of the material"""
    metallicness: float
    """The PBR metallicness of the material"""
    specular: float
    """The PBR specular value of the material"""

    def __init__(self, name: str=None, color: tuple=(255.0, 255.0, 255.0), texture: Image=None, normal: Image=None, roughness: float=16.0, metallicness: float=0.0, specular: float=0.5) -> None:
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
        
        self.index = 0
        
        self.name         = name if name else ''
        self.color        = color
        self.texture      = texture
        self.normal       = normal
        self.roughness    = roughness
        self.metallicness = metallicness
        self.specular     = specular

    def get_data(self) -> list:
        """
        Returns a list containing all the gpu data in the material.
        Used by the material handler
        """

        # Add color and PBR data
        data = [self.color.x / 255.0, self.color.y / 255.0, self.color.z / 255.0, self.roughness, self.metallicness, self.specular]

        # Add texture data
        if self.texture: data.extend([1, self.texture.index.x, self.texture.index.y])
        else: data.extend([0, 0, 0])
        # Add normal data
        if self.normal: data.extend([1, self.normal.index.x, self.normal.index.y])
        else: data.extend([0, 0, 0])

        return data

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
    def roughness(self, value: float | int | glm.float32):
        if isinstance(value, float) or isinstance(value, int):
            self._roughness = float(value)
        elif isinstance(value, glm.float32):
            self._roughness = float(value.value)
        else:
            raise TypeError(f"Material: Invalid roughness value type {type(value)}")

    @metallicness.setter
    def metallicness(self, value: float | int | glm.float32):
        if isinstance(value, float) or isinstance(value, int):
            self._metallicness = float(value)
        elif isinstance(value, glm.float32):
            self._metallicness = float(value.value)
        else:
            raise TypeError(f"Material: Invalid metallicness value type {type(value)}")
        
    @specular.setter
    def specular(self, value: float | int | glm.float32):
        if isinstance(value, float) or isinstance(value, int):
            self._specular = float(value)
        elif isinstance(value, glm.float32):
            self._specular = float(value.value)
        else:
            raise TypeError(f"Material: Invalid specular value type {type(value)}")