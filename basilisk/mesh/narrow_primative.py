import glm
from ..generic.meshes import get_aabb_line_collision

class NarrowPrimative():
    top_right: glm.vec3
    """The furthest positive corner of the AABB"""
    bottom_left: glm.vec3
    """The furthest negative corner of the AABB"""
    geometric_center: glm.vec3
    """The centroid of the primative"""
    index: int
    """the index of the triangle in the mesh"""

    def __init__(self, top_right:glm.vec3, bottom_left:glm.vec3, index: int) -> None:
        self.top_right        = top_right
        self.bottom_left      = bottom_left
        self.geometric_center = (self.top_right + self.bottom_left) / 2
        self.index = index
        
    def is_possible_triangle(self, point: glm.vec3, vec: glm.vec3) -> int:
        """
        Determines if this triangle's AABB intersects with the line
        """
        return self.index if get_aabb_line_collision(self.top_right, self.bottom_left, point, vec) else -1