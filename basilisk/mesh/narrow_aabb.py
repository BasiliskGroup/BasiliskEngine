import glm
from ..generic.abstract_bvh import AbstractAABB as AABB


class NarrowAABB(AABB):
    top_right: glm.vec3
    """The furthest positive corner of the AABB"""
    bottom_left: glm.vec3
    """The furthest negative corner of the AABB"""
    a: ...
    """Child AABB or Collider 1"""
    b: ...
    """Child AABB or Collider 2"""

    def __init__(self, top_right:glm.vec3, bottom_left:glm.vec3, a: AABB, b: AABB):
        self.top_right = top_right
        self.bottom_left = bottom_left
        self.a = a
        self.b = b