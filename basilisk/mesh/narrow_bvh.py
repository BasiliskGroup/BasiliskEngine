import glm
from .narrow_aabb import NarrowAABB
from .narrow_primative import NarrowPrimative
from ..generic.abstract_bvh import AbstractAABB as BVH
from ..generic.meshes import get_extreme_points_np, get_aabb_surface_area


class NarrowBVH(BVH):
    root: NarrowAABB | NarrowPrimative
    """Root aabb used for the start of all collisions"""
    primatives: list[NarrowPrimative]
    """All of the primatives in the BVH associated with triangles in the mesh"""
    mesh: ...
    """Back reference to the parent mesh"""
    
    def __init__(self, mesh) -> None:
        self.mesh       = mesh
        self.primatives = []
        for index, triangle in enumerate(self.mesh.indices):
            points = [self.mesh.points[t] for t in triangle] # TODO check np array accessing
            top_right, bottom_left = get_extreme_points_np(points)
            self.primatives.append(NarrowPrimative(top_right, bottom_left, index))
            
        top_right   = mesh.geometric_center + mesh.half_dimensions
        bottom_left = mesh.geometric_center - mesh.half_dimensions
        self.root   = self.build_bvh(self.primatives, top_right, bottom_left)
        
    def build_bvh(self, primatives: list[NarrowPrimative], top_right: glm.vec3, bottom_left: glm.vec3) -> NarrowAABB | NarrowPrimative:
        """
        Creates a root node for the BVH with the given primatives and bounds
        """
        best_cost  = -1
        best_split = primatives
        best_aabb  = []
        count = len(primatives) // 2
        
        # return primative if it is a leaf
        if not count: return primatives[0]
        
        for axis in range(3):
            # sort primatives along axis and determine if it is lowest cost
            primatives.sort(key=lambda p: p.geometric_center[axis])
            aabb = self.calculate_primative_aabb(primatives[:count]) + self.calculate_primative_aabb(primatives[count:])
            cost = get_aabb_surface_area(aabb[0], aabb[1])  + get_aabb_surface_area(aabb[2], aabb[3])
            
            if best_cost < 0 or cost < best_cost:
                best_cost  = cost
                best_split = list(primatives) # TODO ensure that this is a shallow copy
                best_aabb  = aabb        
        
        a = self.build_bvh(best_split[:count], best_aabb[0], best_aabb[1])
        b = self.build_bvh(best_split[count:], best_aabb[2], best_aabb[3])
        return NarrowAABB(top_right, bottom_left, a, b)
            
    def calculate_primative_aabb(self, primatives: list[NarrowPrimative]) -> float:
        """
        Computes the aabb surface area of the primatives
        """
        points = set()
        for primative in primatives: 
            points.update([tuple(self.mesh.points[t]) for t in self.mesh.indices[primative.index]])
        return list(get_extreme_points_np(list(points)))
    
    def get_possible_triangles(self, point: glm.vec3, vec: glm.vec3) -> list[int]:
        """
        Determines the closest intersecting on the bvh
        """
        if isinstance(self.root, NarrowAABB): return self.root.get_possible_triangles(point, vec)
        index = self.root.is_possible_triangle(point, vec)
        return [index] if index != -1 else []
    
    def get_best_dot(self, vec: glm.vec3) -> int:
        """
        Returns the best triangle with the highest dot product with the vector from the geometric center to its AABB
        """
        if isinstance(self.root, NarrowAABB): return self.root.get_best_dot(vec)
        return self.root.index
    
    def get_all_aabbs(self) -> list[tuple[glm.vec3, glm.vec3, int]]:
        """
        Returns all AABBs, their extreme points, and their layer
        """
        if isinstance(self.root, NarrowAABB): return self.root.get_all_aabbs(0)
        return [(self.root.top_right, self.root.bottom_left, 0)]
    
    def get_line_collided(self, position: glm.vec3, forward: glm.vec3) -> list[tuple[int, int, int]]:
        """
        Determines which triangles are intersecting with the given line segment. Returns the indices of the triangle contained in the mesh points list
        """
        if isinstance(self.root, NarrowAABB): return self.root.get_line_collided(position, forward)
        return self.root.index
        