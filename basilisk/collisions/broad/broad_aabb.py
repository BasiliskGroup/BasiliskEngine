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
    """furthest positive vertex of the AABB"""
    bottom_left: glm.vec3
    """furthest negative vertex of the AABB"""
    
    def __init__(self, a: AABB | Collider, b: AABB | Collider) -> None:
        self.a = a
        self.b = b
        
        # calculate extreme points
        self.top_right   = glm.max(self.a.top_right, self.b.top_right)
        self.bottom_left = glm.min(self.a.bottom_left, self.b.bottom_left)
        
    def find_sibling(self, collider: Collider, parent: AABB, c_best: float, inherited: float) -> tuple[float, AABB, AABB]:
        """
        Determines the best sibling for inserting a collider into the BVH
        """
        # compute lowest cost and determine if children are a viable option
        delta_c = self.surface_area
        c_low   = collider.aabb_surface_area + delta_c + inherited
        c       = collider.aabb_surface_area + inherited
        if c < c_best: c_best = c
        
        # investigate children
        best_aabb, best_parent = self, parent
        if c_low >= c_best: return c_best, best_aabb, best_parent
        for child in (self.a, self.b):
            
            if isinstance(child, BroadAABB): child_c, child_aabb = child.find_sibling(collider, self, c_best, inherited + delta_c)
            else: child_c, child_aabb = child.aabb_surface_area + inherited, child
            if child_c < c_best: 
                c_best      = child_c
                best_aabb   = child_aabb
                best_parent = self
            
        return c_best, best_aabb, best_parent
    
    def get_collided(self, collider: Collider) -> list[Collider]:
        ...
        
    def get_all_aabbs(self, layer: int) -> list[tuple[glm.vec3, glm.vec3, int]]:
        """
        Returns all AABBs, their extreme points, and their layer
        """
        aabbs = [(self.top_right, self.bottom_left, layer)]
        if isinstance(self.a, BroadAABB): aabbs += self.a.get_all_aabbs(layer + 1)
        else: aabbs.append((self.a.top_right, self.a.bottom_left, layer + 1))
        if isinstance(self.b, BroadAABB): aabbs += self.b.get_all_aabbs(layer + 1)
        else: aabbs.append((self.b.top_right, self.b.bottom_left, layer + 1))
        return aabbs
        
    @property
    def surface_area(self): return get_aabb_surface_area(self.top_right, self.bottom_left)
    