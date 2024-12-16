import glm
from .broad_aabb import BroadAABB
from ..collider import Collider
from ...generic.abstract_bvh import AbstractBVH as BVH

class BroadBVH(BVH):
    root: BroadAABB
    """The root node of the BVH"""
    collider_handler: ...
    """Back reference to the collider ahndler for accessing colliders"""
    
    def __init__(self, collider_handler) -> None:
        self.collider_handler = collider_handler
        
    def add(self, collider):
        """
        Adds a single collider to the bvh tree
        """
        # test if tree needs to be initiated
        if not self.root: 
            self.root = collider # TODO ensure that this is final format for primative
            return
        
        # check if root is primative
        if isinstance(self.root, Collider): 
            self.root = BroadAABB(self.root, collider)
            return
        
        # find the best sibling (c_best only used during the recursion)
        c_best, sibling, old_parent = self.root.find_sibling(collider, None, 1e10, 0)
        new_parent = BroadAABB(sibling, collider)
        
        # if the sibling was not the root
        if old_parent:
            if old_parent.a == sibling: old_parent.a = new_parent
            else:                       old_parent.b = new_parent
        else: self.root = new_parent
        
    def remove(self, collider): ...
    
    def rotate(self): ...
        
    def get_collided(self, collider): ... 