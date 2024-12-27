import glm
from .collider import Collider
from .broad.broad_bvh import BroadBVH
from ..mesh.cube import cube
from ..generic.collisions import get_sat_axes

class ColliderHandler():
    scene: ...
    """Back reference to scene"""
    colliders: list[Collider]
    """Main list of collders contained in the scene"""
    bvh: BroadBVH
    """Broad bottom up BVH containing all colliders in the scene"""
    
    def __init__(self, scene) -> None:
        self.scene = scene
        self.colliders = []
        self.bvh = BroadBVH(self)
        
    def add(self, node, box_mesh: bool=False, static_friction: glm.vec3=0.7, kinetic_friction: glm.vec3=0.3, elasticity: glm.vec3=0.1, collision_group: str=None) -> Collider:
        """
        Creates a collider and adds it to the collider list
        """
        collider = Collider(self, node, box_mesh, static_friction, kinetic_friction, elasticity, collision_group)
        self.colliders.append(collider)
        self.bvh.add(collider)
        return collider
    
    def remove(self, collider: Collider) -> None:
        """
        Removes a collider from the main branch and BVH
        """
        if collider in self.colliders: self.colliders.remove(collider)
        self.bvh.remove(collider)
        collider.collider_handler = None
    
    def resolve_collisions(self) -> None:
        """
        Resets collider collision values and resolves all collisions in the scene
        """
        for collider in self.colliders: collider.collisions = {}
        
    def collide_obb_obb(self, collider1: Collider, collider2: Collider) -> tuple[glm.vec3, float] | None:
        """
        Finds the minimal penetrating vector for an obb obb collision, return None if not colliding. Uses SAT. 
        """
        axes = get_sat_axes(collider1.node.rotation, collider2.node.rotation)     
        points1 = collider1.obb_points # TODO remove once oobb points are lazy updated, switch to just using property
        points2 = collider2.obb_points
                
        # test axes
        small_axis    = None
        small_overlap = 1e10
        for axis in axes: # TODO add optimization for points on cardinal axis of cuboid
            # "project" points
            proj1 = [glm.dot(p, axis) for p in points1]
            proj2 = [glm.dot(p, axis) for p in points2]
            max1, min1 = max(proj1), min(proj1)
            max2, min2 = max(proj2), min(proj2)
            if max1 < min2 or max2 < min1: return None
            
            # if lines are not intersecting
            overlap = min(max1, max2) - max(min1, min2)
            if overlap < 0 or overlap > small_overlap: continue
            small_overlap = overlap
            small_axis    = axis
            
        return small_axis, small_overlap
    
    def collide_obb_obb_decision(self, collider1: Collider, collider2: Collider) -> bool:
        """
        Determines if two obbs are colliding Uses SAT. 
        """
        axes = get_sat_axes(collider1.node.rotation, collider2.node.rotation)     
        points1 = collider1.obb_points # TODO remove once oobb points are lazy updated, switch to just using property
        points2 = collider2.obb_points
                
        # test axes
        for axis in axes: # TODO add optimization for points on cardinal axis of cuboid
            # "project" points
            proj1 = [glm.dot(p, axis) for p in points1]
            proj2 = [glm.dot(p, axis) for p in points2]
            max1, min1 = max(proj1), min(proj1)
            max2, min2 = max(proj2), min(proj2)
            if max1 < min2 or max2 < min1: return False
            
        return True
    
    def resolve_broad_collisions(self) -> set[tuple[Collider, Collider]]:
        """
        Determines which colliders collide with each other from the BVH
        """
        collisions = set()
        for collider1 in self.colliders:
            
            # traverse bvh to find aabb aabb collisions
            colliding = self.bvh.get_collided(collider1)
            for collider2 in colliding:
                if collider1 == collider2: continue
                if (collider1, collider2) in collisions or (collider2, collider1) in collisions: continue # TODO find a secure way for ordering colliders
                
                # run broad collision for specified mesh types
                if max(len(collider1.mesh.points), len(collider2.mesh.points)) > 250:  # contains at least one "large" mesh TODO write heuristic algorithm for determining large meshes
                    ...
                
                collisions.add((collider1, collider2)) # TODO find a secure way for ordering colliders
    
    def resolve_narrow_collisions(self): ...