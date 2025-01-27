import glm
from .broad_aabb import BroadAABB
from ..collider import Collider
from ...generic.abstract_bvh import AbstractBVH as BVH
from ...generic.meshes import get_aabb_surface_area

class BroadBVH(BVH):
    root: BroadAABB
    """The root node of the BVH"""
    collider_handler: ...
    """Back reference to the collider ahndler for accessing colliders"""
    
    def __init__(self, collider_handler) -> None:
        self.collider_handler = collider_handler
        self.root = None
        
    def add(self, collider: Collider) -> None:
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
            self.rotate(aabb)
            aabb = aabb.parent
        
    def get_all_aabbs(self) -> list[tuple[glm.vec3, glm.vec3, int]]: # TODO test function
        """
        Returns all AABBs, their extreme points, and their layer
        """
        if isinstance(self.root, BroadAABB): return self.root.get_all_aabbs(0)
        return [(self.root.top_right, self.root.bottom_left, 0)]
        
    def remove(self, collider: Collider) -> None:
        """
        Removes a collider from the BVH, refitting the tree and adjusting relations
        """
        parent: BroadAABB | None = collider.parent
        
        # if collider is the root, remove the root
        if not parent:
            self.root = None
            return
        
        # if collider has no grandparent, remove parent and set sibling as root
        grand   = parent.parent
        sibling = parent.b if collider == parent.a else parent.a
        if not grand:
            self.root = sibling
            sibling.parent = None
            return
    
        # if grandparent exists
        if parent == grand.a: grand.a = sibling
        else:                 grand.b = sibling
        sibling.parent = grand
        
        # move up and refit tree
        aabb = grand
        while aabb:
            aabb.update_points()
            aabb = aabb.parent
    
    def rotate(self, aabb: BroadAABB) -> None:
        """
        Rotates the BVH tree to reduce surface area of internal AABBs
        """
        # determine if rotation is possible
        parent: BroadAABB | None = aabb.parent
        if not parent: return
        
        grand = parent.parent
        if not grand: return
        
        # determine if swapping 
        aunt = grand.b if grand.a == parent else grand.a
        sibling = parent.b if parent.a == aabb else parent.a
        
        top_right   = glm.max(aunt.top_right, sibling.top_right)
        bottom_left = glm.min(aunt.bottom_left, sibling.bottom_left)
        aunt_sibling_area = get_aabb_surface_area(top_right, bottom_left)
        
        if aunt_sibling_area > parent.surface_area: return
        
        # rotate tree if necessary
        if grand.a == aunt: grand.a = aabb
        else:               grand.b = aabb
        
        if parent.a == aabb: parent.a = aunt
        else:                parent.b = aunt
        
        # reset parents and update points to resize AABBs
        aunt.parent = parent
        aabb.parent = grand
        
        parent.update_points()
        grand.update_points()
        
    def get_collided(self, collider: Collider) -> list[Collider]:
        """
        Returns which objects may be colliding from the BVH
        """
        if isinstance(self.root, BroadAABB): return self.root.get_collided(collider)
        else: return [] # if there is only one collider in the scene then there is nothing to collide with
        
    def get_line_collided(self, position: glm.vec3, forward: glm.vec3) -> list[Collider]:
        """
        Returns the colliders that may intersect with the given line
        """
        if isinstance(self.root, BroadAABB): return self.root.get_line_collided(position, forward)
        return [self.root]