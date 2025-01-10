import glm
import os
from .mesh import Mesh

    
class Cube(Mesh):
    def __init__(self, engine) -> None:
        # built-in cube mesh with custom functions
        path = engine.root + '/bsk_assets/cube.obj'
        super().__init__(path)

    def get_best_dot(self, vec: glm.vec3) -> glm.vec3:
        """
        Gets the best dot point of a cube
        """
        return glm.vec3([-1 if v < 0 else 1 for v in vec])