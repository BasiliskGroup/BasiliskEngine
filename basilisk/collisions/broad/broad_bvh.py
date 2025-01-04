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
        self.root = None
        
    def add(self, collider: Collider):
        """
        Adds a single collider to the bvh tree
        """
        # test if tree needs to be initiated
        if not self.root: 
            self.root = collider # TODO ensure that this is final format for primative
            return
        
        # check if root is primative
        if isinstance(self.root, Collider): 
            sibling = self.root
            self.root      = BroadAABB(sibling, collider, None)
            sibling.parent = collider.parent = self.root
            return
        
        # find the best sibling (c_best only used during the recursion)
        c_best, sibling = self.root.find_sibling(collider, 0)
        old_parent = sibling.parent
        new_parent = BroadAABB(sibling, collider, old_parent)
        
        # if the sibling was not the root
        if old_parent:
            if old_parent.a == sibling: old_parent.a = new_parent
            else:                       old_parent.b = new_parent
        else: self.root = new_parent
        
        sibling.parent = new_parent
        collider.parent = new_parent
        
        # walk back up tree and refit TODO add tree rotations
        aabb = new_parent
        while aabb:
            aabb.update_points()
            aabb = aabb.parent
        
    def get_all_aabbs(self) -> list[tuple[glm.vec3, glm.vec3, int]]: # TODO test function
        """
        Returns all AABBs, their extreme points, and their layer
        """
        if isinstance(self.root, BroadAABB): return self.root.get_all_aabbs(0)
        return [(self.root.top_right, self.root.bottom_left, 0)]
        
    def remove(self, collider: Collider): ...
    
    def rotate(self): ...
        
    def get_collided(self, collider: Collider):
        """
        Returns which objects may be colliding from the BVH
        """
        return self.root.get_collided(collider)