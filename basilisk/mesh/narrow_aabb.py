import glm

from basilisk.generic.collisions import collide_aabb_line
from .narrow_primative import NarrowPrimative
from ..generic.abstract_bvh import AbstractAABB as AABB
from ..generic.meshes import get_aabb_line_collision

class NarrowAABB(AABB):
    top_right: glm.vec3
    """The furthest positive corner of the AABB"""
    bottom_left: glm.vec3
    """The furthest negative corner of the AABB"""
    geometric_center: glm.vec3
    """The center of the object calculated from its extreme points"""
    a: AABB | NarrowPrimative
    """Child AABB or Collider 1"""
    b: AABB | NarrowPrimative
    """Child AABB or Collider 2"""

    def __init__(self, top_right:glm.vec3, bottom_left:glm.vec3, a: AABB, b: AABB) -> None:
        self.top_right = top_right
        self.bottom_left = bottom_left
        self.geometric_center = (top_right + bottom_left) / 2
        self.a = a
        self.b = b
        
    def get_possible_triangles(self, point: glm.vec3, vec: glm.vec3) -> list[int]:
        """
        Determines the closest intersecting on the bvh
        """
        indices = []
        if not get_aabb_line_collision(self.top_right, self.bottom_left, point, vec): return indices
        
        for child in (self.a, self.b):
            
            # if child is another AABB
            if isinstance(child, NarrowAABB):
                indices += child.get_possible_triangles(point, vec)
                continue
            
            # if child is a primative
            index = child.is_possible_triangle(point, vec)
            if index == -1: continue
            indices.append(index)
        
        return indices
    
    def get_best_dot(self, vec: glm.vec3) -> int:
        """
        Returns the best triangle with the highest dot product with the vector from the geometric center to its AABB
        """
        c = max(self.a, self.b, key=lambda x: glm.dot(x.geometric_center, vec))
        if isinstance(c, NarrowAABB): return c.get_best_dot(vec)
        return c.index
    
    def get_all_aabbs(self, layer: int) -> list[tuple[glm.vec3, glm.vec3, int]]:
        """
        Returns all AABBs, their extreme points, and their layer
        """
        aabbs = [(self.top_right, self.bottom_left, layer)]
        if isinstance(self.a, NarrowAABB): aabbs += self.a.get_all_aabbs(layer + 1)
        else: aabbs.append((self.a.top_right, self.a.bottom_left, layer + 1))
        if isinstance(self.b, NarrowAABB): aabbs += self.b.get_all_aabbs(layer + 1)
        else: aabbs.append((self.b.top_right, self.b.bottom_left, layer + 1))
        return aabbs
    
    def get_tested_aabbs(self, point: glm.vec3, vec: glm.vec3, layer: int) -> list[tuple[glm.vec3, glm.vec3, int]]:
        """
        Returns all AABBs, their extreme points, and their layer
        """
        aabbs = [(self.top_right, self.bottom_left, layer)]
        
        if isinstance(self.a, NarrowAABB): 
            
            aabbs += self.a.get_all_aabbs(layer + 1)
        else: aabbs.append((self.a.top_right, self.a.bottom_left, layer + 1))
        
        if isinstance(self.b, NarrowAABB): 
            
            aabbs += self.b.get_all_aabbs(layer + 1)
        else: aabbs.append((self.b.top_right, self.b.bottom_left, layer + 1))
        
        return aabbs
    
    def get_line_collided(self, position: glm.vec3, forward: glm.vec3) -> list[int]:
        """
        Returns the colliders that may intersect with the given line
        """
        if not collide_aabb_line(self.top_right, self.bottom_left, position, forward): return []
        return (self.a.get_line_collided(position, forward) if isinstance(self.a, NarrowAABB) else [self.a.index]) + (self.b.get_line_collided(position, forward) if isinstance(self.b, NarrowAABB) else [self.b.index])