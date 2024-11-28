import glm
import numpy as np


class Light():
    light_handler: ...
    """Back reference to the parent light handler"""
    intensity: float
    """The brightness of the light"""
    color: glm.vec3
    """The color of the light"""

    def __init__(self, light_handler, intensity: float=1.0, color: tuple=(255, 255, 255)):
        """
        Abstract light class for Basilisk Engine.
        Cannot be added to a scene.
        """

        # Back References
        self.light_handler = light_handler

        # Light attributes
        self.intensity = intensity
        self.color = color

    @property 
    def intensity(self): return self._intensity
    @property
    def color(self):     return self._color

    @intensity.setter
    def intensity(self, value: float | int):
        if isinstance(value, float) or isinstance(value, int):
            self._intensity = value
        else:
            raise TypeError(f"Light: Invalid intensity value type {type(value)}. Expected float or int")
        self.light_handler.write()

    @color.setter
    def color(self, value: tuple | list | glm.vec3 | np.ndarray):
        if isinstance(value, tuple) or isinstance(value, list) or isinstance(value, np.ndarray):
            if len(value) != 3: raise ValueError(f"Light: Invalid number of values for color. Expected 3 values, got {len(value)} values")
            self._color = glm.vec3(value)
        elif isinstance(value, glm.vec3):
            self._color = glm.vec3(value)
        else:
            raise TypeError(f"Light: Invalid color value type {type(value)}. Expected tuple, list, glm.vec3, or numpy array")
        self.light_handler.write()

class DirectionalLight(Light):
    direction: glm.vec3
    """The direction that the light is applied to objects"""
    ambient: float
    """Base value of light that is applied at all locations, regardless of direction"""

    def __init__(self, light_handler, direction: tuple=(1.5, -2.0, 1.0), intensity:float=1.0, color: tuple=(255, 255, 255), ambient: float=0.0):
        """
        Diractional/Global light for Basilisk Engine.
        Has same intensity and direction everywhere.
        Args:
            direction: tuple
                The direction that the light is applied to objects
            intensity: float
                The brightness of the light
            color: tuple
                The color of the light
            ambient: float
                Base value of light that is applied at all locations, regardless of direction
        """
        
        super().__init__(light_handler, intensity, color)
        self.direction = direction
        self.ambient = ambient

    @property
    def direction(self): return self._direction
    @property
    def ambient(self):   return self._ambient

    @direction.setter
    def direction(self, value: tuple | list | glm.vec3 | np.ndarray):
        if isinstance(value, tuple) or isinstance(value, list) or isinstance(value, np.ndarray):
            if len(value) != 3: raise ValueError(f"Light: Invalid number of values for direction. Expected 3 values, got {len(value)} values")
            self._direction = glm.normalize(glm.vec3(value))
        elif isinstance(value, glm.vec3):
            self._direction = glm.normalize(glm.vec3(value))
        else:
            raise TypeError(f"Light: Invalid direction value type {type(value)}. Expected tuple, list, glm.vec3, or numpy array")
        self.light_handler.write()
        
    @ambient.setter
    def ambient(self, value: float | int):
        if isinstance(value, float) or isinstance(value, int):
            self._ambient = value
        else:
            raise TypeError(f"Light: Invalid ambient value type {type(value)}. Expected float or int")
        self.light_handler.write()