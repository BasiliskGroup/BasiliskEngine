import glm
import os
from .mesh import Mesh

    
class Cube(Mesh):
    def __init__(self) -> None:
        # built-in cube mesh with custom functions
        dire = os.path.dirname(__file__)
        path = os.path.join(dire, 'built-in', 'cube.obj')
        super().__init__(path)

    def get_best_dot(self, vec: glm.vec3) -> glm.vec3:
        """
        Gets the best dot point of a cube
        """
        return glm.vec3([-1 if v < 0 else 1 for v in vec])
    
# create instance of cube mesh to be used by both the user and the package. Needs to be the same cube object for internal comparisons. Do not allow the user to access the Cube class to prevent them from making other Cube objects. 
cube = Cube()