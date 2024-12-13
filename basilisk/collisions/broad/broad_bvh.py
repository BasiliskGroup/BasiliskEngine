import glm
from .broad_aabb import BroadAABB
from ...generic.abstract_bvh import AbstractBVH as BVH

class BroadBVH(BVH):
    root: BroadAABB
    """The root node of the BVH"""
    collider_handler: ...
    """Back reference to the collider ahndler for accessing colliders"""
    
    def __init__(self, collider_handler) -> None:
        self.collider_handler = collider_handler
        
    def build_tree(self) -> None:
        """
        Generates BVH tree from collider handler
        """
        colliders = self.collider_handler.colliders
        
        if not len(colliders): 
            self.root = None
            return
        
        if len(colliders) == 1: 
            self.root = colliders[0] # TODO ensure that this is final format for primative
            return
        
        for collider in colliders: self.add(collider)
        
    def add(self, collider): ...
    
    def remove(self, collider): ...
    
    def rotate(self): ...
        
    def get_collided(self, collider): ... 