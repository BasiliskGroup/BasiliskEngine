import glm
from .collider import Collider
from .broad.broad_bvh import BroadBVH

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
    
    def resolve_broad_collisions(self): ...
    
    def resolve_narrow_collisions(self): ...