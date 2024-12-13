import glm
from ...generic.abstract_bvh import AbstractAABB as AABB
from ...generic.collisions import collide_aabb_aabb
from ...generic.meshes import get_aabb_surface_area
from ..collider import Collider


class BroadAABB(AABB):
    a: AABB | Collider
    """The first child of the AABB"""
    b: AABB | Collider
    """The second child of the AABB"""
    top_right: glm.vec3
    """The furthest positive point of the AABB"""
    bottom_left: glm.vec3
    """The furthest negative point of the AABB"""
    
    def __init__(self, a: AABB, b: AABB) -> None:
        self.a = a
        self.b = b
        self.top_right   = glm.max(self.a.top_right, self.b.top_right)
        self.bottom_left = glm.min(self.a.bottom_left, self.b.bottom_left)
        
    def find_sibling(self, collider: Collider, c_best: float, inherited: float) -> tuple[float, AABB]:
        """
        Determines the best sibling for inserting a collider into the BVH
        """
        # compute lowest cost and determine if children are a viable option
        delta_c = get_aabb_surface_area(self.top_right, self.bottom_left)
        c_low   = collider.surface_area + delta_c + inherited
        c       = collider.surface_area + inherited
        if c < c_best: c_best = c
        
        # investigate children
        best_aabb = self
        if c_low >= c_best: return c_best, best_aabb
        for child in (self.a, self.b):
            
            if isinstance(child, BroadAABB): child_c, child_aabb = child.find_sibling(collider, c_best, inherited + delta_c)
            else: child_c, child_aabb = child.surface_area + inherited, child
            if child_c < c_best: c_best, best_aabb = child_c, child_aabb
            
        return c_best, best_aabb
    
    def get_collided(self, collider: Collider) -> list[Collider]:
        ...
        