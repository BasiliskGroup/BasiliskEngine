import glm

from .collider import Collider
from .broad.broad_bvh import BroadBVH
from .narrow.gjk import collide_gjk
from .narrow.epa import get_epa_from_gjk
from .narrow.contact_manifold import get_contact_manifold, separate_polytope
from .narrow.dataclasses import ContactPoint, ContactManifold, Collision
from ..nodes.node import Node
from ..generic.collisions import get_sat_axes
from ..physics.impulse import calculate_collisions

class ColliderHandler():
    scene: ...
    """Back reference to scene"""
    colliders: list[Collider]
    """Main list of collders contained in the scene"""
    bvh: BroadBVH
    """Broad bottom up BVH containing all colliders in the scene"""
    
    def __init__(self, scene) -> None:
        self.scene = scene
        self.cube = self.scene.engine.cube
        self.colliders = []
        self.polytope_data = {}
        self.contact_manifolds: dict[tuple[Collider, Collider] : ContactManifold] = {}
        self.bvh = BroadBVH(self)
        
    def add(self, collider: Collider) -> Collider:
        """
        Creates a collider and adds it to the collider list
        """
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
        # reset collision data
        for collider in self.colliders: collider.collisions = []
        
        # update BVH
        for collider in self.colliders:
            if collider.needs_bvh:
                self.bvh.remove(collider)
                self.bvh.add(collider)
                collider.needs_bvh = False
        
        # resolve collisions
        broad_collisions = self.resolve_broad_collisions()
        self.resolve_narrow_collisions(broad_collisions) 
        
    def collide_obb_obb(self, collider1: Collider, collider2: Collider) -> tuple[glm.vec3, float] | None:
        """
        Finds the minimal penetrating vector for an obb obb collision, return None if not colliding. Uses SAT. 
        """
        axes = get_sat_axes(collider1.node.rotation, collider2.node.rotation) # axes are normaized
        points1 = collider1.obb_points # TODO remove once oobb points are lazy updated, switch to just using property
        points2 = collider2.obb_points
                
        # test axes
        small_axis    = None
        small_overlap = 1e10
        small_index = 0
        for i, axis in enumerate(axes): # TODO add optimization for points on cardinal axis of cuboid
            # "project" points
            proj1 = [glm.dot(p, axis) for p in points1]
            proj2 = [glm.dot(p, axis) for p in points2]
            max1, min1 = max(proj1), min(proj1)
            max2, min2 = max(proj2), min(proj2)
            if max1 < min2 or max2 < min1: return None
            
            # if lines are not intersecting
            if   max1 > max2 and min1 < min2: overlap = min(max1 - min2, max2 - min1)
            elif max2 > max1 and min2 < min1: overlap = min(max2 - min1, max1 - min2)
            else:                             overlap = min(max1, max2) - max(min1, min2) # TODO check if works with containment
            
            if abs(overlap) > abs(small_overlap): continue
            small_overlap = overlap
            small_axis    = axis
            small_index   = i
            
        return small_axis, small_overlap, small_index
    
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
            if collider1.node.static: continue
            # traverse bvh to find aabb aabb collisions
            colliding = self.bvh.get_collided(collider1)
            for collider2 in colliding:
                if collider1 is collider2 or (collider1.collision_group is not None and collider1.collision_group == collider2.collision_group): continue
                if ((collider1, collider2) if id(collider1) < id(collider2) else (collider2, collider1)) in collisions: continue
                
                # run broad collision for specified mesh types
                if max(len(collider1.mesh.points), len(collider2.mesh.points)) > 250 and not self.collide_obb_obb_decision(collider1, collider2): continue # contains at least one "large" mesh TODO write heuristic algorithm for determining large meshes
                collisions.add((collider1, collider2) if id(collider1) < id(collider2) else (collider2, collider1))
                
        return collisions
    
    def merge_contact_points(self, vec: glm.vec3, collider1: Collider, collider2: Collider, points1: list[ContactPoint], points2: list[ContactPoint]) -> None:
        """
        
        """
        def merge_points(node: Node, existing: dict[int, glm.vec3], incoming: list[ContactPoint]) -> dict[int, glm.vec3]:
            incoming_indices = set()
            
            # add incoming points
            for point in incoming:
                incoming_indices.add(point.index)
                if point.index not in existing or glm.length2(point.vertex - existing[point.index]) > 1e-5: existing[point.index] = glm.vec3(point.vertex)
                    
            # remove changed stored points
            remove_indices = []
            for index, vertex in existing.items():
                if index in incoming_indices: continue
                if glm.length2(node.collider.get_vertex(index) - vertex) > 1e-5: remove_indices.append(index) # check to see if point has moved
            
            # remove unused and moved points
            for index in remove_indices: del existing[index]
            return existing
        
        # check if collision is logged, if not create a new one
        collider_tuple = (collider1, collider2)
        if collider_tuple not in self.contact_manifolds or glm.length2(self.contact_manifolds[collider_tuple].normal - vec) > 1e-7: self.contact_manifolds[collider_tuple] = ContactManifold(vec, dict(), dict())
        
        # add contact point from current collision and check overlap
        self.contact_manifolds[collider_tuple].contact_points1 = merge_points(collider1.node, self.contact_manifolds[collider_tuple].contact_points1, points1)
        self.contact_manifolds[collider_tuple].contact_points2 = merge_points(collider2.node, self.contact_manifolds[collider_tuple].contact_points2, points2)
    
    def resolve_narrow_collisions(self, broad_collisions: list[tuple[Collider, Collider]]) -> None:
        """
        Determines if two colliders are colliding, if so resolves their penetration and applies impulse
        """
        for collision in broad_collisions: # assumes that broad collisions are unique
            collider1 = collision[0]
            collider2 = collision[1]
            node1: Node = collider1.node
            node2: Node = collider2.node
            
            # get peneration data or quit early if no collision is found
            if collider1.mesh == self.cube and collider2.mesh == self.cube: # obb-obb collision
                
                # run SAT for obb-obb (includes peneration)
                data = self.collide_obb_obb(collider1, collider2)
                if not data: continue
                
                vec, distance, index = data
                
                # TODO replace with own contact algorithm
                points1 = [ContactPoint(index, vertex) for index, vertex in enumerate(collider1.obb_points)]
                points2 = [ContactPoint(index, vertex) for index, vertex in enumerate(collider2.obb_points)]
                
            else: # use gjk to determine collisions between non-cuboid meshes
                has_collided, simplex = collide_gjk(node1, node2)
                if not has_collided: continue
                
                face, polytope = get_epa_from_gjk(node1, node2, simplex)
                vec, distance  = face[1], face[0]
                
                # TODO replace with own contact algorithm
                points1 = [ContactPoint(p.index1, p.vertex1) for p in polytope]
                points2 = [ContactPoint(p.index2, p.vertex2) for p in polytope]
                
            if glm.dot(vec, node2.position.data - node1.position.data) > 0: vec *= -1
            
            # add collision data to colliders
            collider1.collisions.append(Collision(node2, vec))
            collider2.collisions.append(Collision(node1, -vec))
            
            # apply impulse if a collider has a physic body
            if node1.physics_body or node2.physics_body:
                
                # determine the contact points from the collision
                points1, points2 = separate_polytope(points1, points2, vec)
                self.merge_contact_points(vec, collider1, collider2, points1, points2)
                
                collider_tuple = (collider1, collider2)
                manifold = get_contact_manifold(
                    node1.position.data - vec, 
                    vec, 
                    self.contact_manifolds[collider_tuple].contact_points1.values(), 
                    self.contact_manifolds[collider_tuple].contact_points2.values()
                )
                
                collision_normal = node1.velocity - node2.velocity
                collision_normal = vec if glm.length2(collision_normal) < 1e-12 else glm.normalize(collision_normal)
                calculate_collisions(collision_normal, node1, node2, manifold, node1.get_inverse_inertia(), node2.get_inverse_inertia(), node1.center_of_mass, node2.center_of_mass)
            
            # resolve collision penetration
            multiplier = 0.5 if not (node1.static or node2.static) else 1
            if not node1.static: node1.position += multiplier * vec * distance
            if not node2.static: node2.position -= multiplier * vec * distance