import glm
from ...generic.abstract_bvh import AbstractAABB as AABB
from ...generic.collisions import collide_aabb_aabb, collide_aabb_line
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
    parent: AABB
    """Back reference to the parent AABB"""
    
    def __init__(self, a: AABB | Collider, b: AABB | Collider, parent: AABB) -> None:
        self.a = a
        self.b = b
        self.parent = parent
        
        # calculate extreme points
        self.update_points()
        
    def update_points(self) -> None:
        """
        Updates the extreme points of the AABB based on the children
        """
        self.top_right   = glm.max(self.a.top_right, self.b.top_right)
        self.bottom_left = glm.min(self.a.bottom_left, self.b.bottom_left)
        
    def find_sibling(self, collider: Collider, inherited: float) -> tuple[float, AABB | Collider]:
        """
        Determines the best sibling for inserting a collider into the BVH
        """
        # compute estimate sa
        top_right   = glm.max(self.top_right, collider.top_right)
        bottom_left = glm.min(self.bottom_left, collider.bottom_left)
        union_area  = get_aabb_surface_area(top_right, bottom_left)
        
        # compute lowest cost and determine if children are a viable option
        c_best = union_area + inherited
        
        delta_surface_area = union_area - self.surface_area 

        c_low = collider.aabb_surface_area + delta_surface_area + inherited
        
        # investigate children
        best_sibling = self
        if c_low >= c_best: return c_best, best_sibling
        
        for child in (self.a, self.b):
            if isinstance(child, BroadAABB): child_c, child_aabb = child.find_sibling(collider, inherited + delta_surface_area)
            else: 
                # compute cost for child
                top_right   = glm.max(self.top_right, child.top_right)
                bottom_left = glm.min(self.bottom_left, child.bottom_left)
                union_area  = get_aabb_surface_area(top_right, bottom_left)

                child_c, child_aabb = union_area + inherited, child
            
            if child_c < c_best: c_best, best_sibling = child_c, child_aabb
            
        return c_best, best_sibling
    
    def get_collided(self, collider: Collider) -> list[Collider]:
        """
        Returns which objects may be colliding from the BVH
        """
        if not collide_aabb_aabb(self.top_right, self.bottom_left, collider.top_right, collider.bottom_left): return []
        
        # test children
        possible = []
        if isinstance(self.a, BroadAABB): possible.extend(self.a.get_collided(collider))
        elif collide_aabb_aabb(self.a.top_right, self.a.bottom_left, collider.top_right, collider.bottom_left): possible.append(self.a)
        if isinstance(self.b, BroadAABB): possible.extend(self.b.get_collided(collider))
        elif collide_aabb_aabb(self.b.top_right, self.b.bottom_left, collider.top_right, collider.bottom_left): possible.append(self.b)
        return possible
    
    def get_line_collided(self, position: glm.vec3, forward: glm.vec3) -> list[Collider]:
        """
        Returns the colliders that may intersect with the given line
        """
        if not collide_aabb_line(self.top_right, self.bottom_left, position, forward): return []
        return (self.a.get_line_collided(position, forward) if isinstance(self.a, BroadAABB) else [self.a]) + (self.b.get_line_collided(position, forward) if isinstance(self.b, BroadAABB) else [self.b])
        
    def get_all_aabbs(self, layer: int) -> list[tuple[glm.vec3, glm.vec3, int]]: # TODO test function
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
    