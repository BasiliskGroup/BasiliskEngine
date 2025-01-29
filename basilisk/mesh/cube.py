import glm
import os
from .mesh import Mesh

    
class Cube(Mesh):
    def __init__(self, engine) -> None:
        # built-in cube mesh with custom functions
        path = engine.root + '/bsk_assets/cube.obj'
        super().__init__(path)
        
        self.dot_indices = [0 for _ in range(8)]
        for i, point in enumerate(self.points):
            index = 0 
            if point[0] > 0: index += 4
            if point[1] > 0: index += 2
            if point[2] > 0: index += 1
            self.dot_indices[index] = i

    def get_best_dot(self, vec: glm.vec3) -> int:
        """
        Gets the best dot point of a cube
        """
        index = 0
        if vec[0] > 0: index += 4
        if vec[1] > 0: index += 2
        if vec[2] > 0: index += 1
        return self.dot_indices[index]
    
    def get_line_collided(self, position: glm.vec3, forward: glm.vec3) -> list[int]:
        """
        Returns all the faces on the cube since the AABB degenerates on the cube mesh
        """
        return [i for i in range(12)]