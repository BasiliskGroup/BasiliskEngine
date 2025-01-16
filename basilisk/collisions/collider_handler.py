import glm

from basilisk.collisions.narrow.graham_scan import graham_scan
from basilisk.collisions.narrow.sutherland_hodgman import sutherland_hodgman
from .collider import Collider
from .broad.broad_bvh import BroadBVH
from .narrow.gjk import collide_gjk
from .narrow.epa import get_epa_from_gjk
from .narrow.contact_manifold import get_contact_manifold, points_to_2d, points_to_3d, project_points
from .narrow.line_intersections import closest_two_lines, line_poly_intersect
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
        # reset collision data
        for collider in self.colliders: collider.collisions = {}
        # update BVH
        for collider in self.colliders:
            if collider.needs_bvh:
                self.bvh.remove(collider)
                self.bvh.add(collider)
        
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
            
        if small_overlap < 0: print(small_overlap)
            
        return small_axis, small_overlap, small_index
    
    def sat_manifold(self, points1: list[glm.vec3], points2: list[glm.vec3], axis: glm.vec3, plane_point: glm.vec3, digit: int) -> list[glm.vec3]:
        """
        Returns the contact manifold from an SAT OBB OBB collision
        """
        def get_test_points(contact_plane_normal:glm.vec3, points:list[glm.vec3], count: int):
            test_points = [(glm.dot(contact_plane_normal, p), p) for p in points]
            test_points.sort(key=lambda p: p[0])
            return [p[1] for p in test_points[:count]]
        
        def get_test_points_unknown(contact_plane_normal:glm.vec3, points:list[glm.vec3]):
            test_points = [(glm.dot(contact_plane_normal, p), p) for p in points]
            test_points.sort(key=lambda p: p[0])
            if test_points[2][0] - test_points[0][0] > 1e-3: return [p[1] for p in test_points[:2]]
            else:                                            return [p[1] for p in test_points[:4]]        
        
        if digit < 6: # there must be at least one face in the collision
            reference, incident = (get_test_points(-axis, points1, 4), get_test_points_unknown(axis, points2)) if digit < 3 else (get_test_points(axis, points2, 4), get_test_points_unknown(-axis, points1))
            
            # project vertices onto the 2d plane
            reference = project_points(plane_point, axis, reference)
            incident  = project_points(plane_point, axis, incident)
            
            # convert points to 2d for intersection algorithms
            reference, u1, v1 = points_to_2d(plane_point, axis, reference)
            incident,  u2, v2 = points_to_2d(plane_point, axis, incident, u1, v1) #TODO precalc orthogonal basis for 2d conversion
            
            # convert arbitrary points to polygon
            reference = graham_scan(reference)
            if len(incident) == 4:  incident =  graham_scan(incident)
            
            # run clipping algorithms
            manifold = []
            if len(incident) == 2: manifold = line_poly_intersect(incident, reference)
            else:                  manifold = sutherland_hodgman(reference, incident)
                
            # # fall back if manifold fails to develope
            assert len(manifold), 'sat did not generate points'
            
            # # convert inertsection algorithm output to 3d
            return points_to_3d(u1, v1, plane_point, manifold)
        
        else: # there is an edge edge collision
            
            points1 = get_test_points(-axis, points1, 2)
            points2 = get_test_points(axis, points2, 2)
            
            return closest_two_lines(*points1, *points2)
    
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
                if max(len(collider1.mesh.points), len(collider2.mesh.points)) > 250 and not self.collide_obb_obb_decision(collider1, collider2): continue # contains at least one "large" mesh TODO write heuristic algorithm for determining large meshes
                
                collisions.add((collider1, collider2)) # TODO find a secure way for ordering colliders
                
        return collisions
    
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
                points1 = collider1.obb_points
                points2 = collider2.obb_points
                
            else: # use gjk to determine collisions between non-cuboid meshes
                has_collided, simplex = collide_gjk(node1, node2)
                if not has_collided: continue
                
                face, polytope = get_epa_from_gjk(node1, node2, simplex)
                vec, distance  = face[1], face[0]
                
                # TODO replace with own contact algorithm
                points1 = [p[1] for p in polytope]
                points2 = [p[2] for p in polytope]
                
            if glm.dot(vec, node2.position - node1.position) > 0: vec *= -1
            manifold = self.sat_manifold(points1, points2, vec, node1.position - vec, index)
            
            print(vec)
            if node1.physics_body or node2.physics_body:
                manifold = get_contact_manifold(node1.position - vec, vec, points1, points2)
                if len(manifold) == 0: 
                    print('manifold failed to generate')
                    continue
                calculate_collisions(vec, node1, node2, manifold, node1.get_inverse_inertia(), node2.get_inverse_inertia(), node1.geometric_center, node2.geometric_center)
            
            # resolve collision penetration
            multiplier = 0.5 if not (node1.static or node2.static) else 1
            if not node1.static: node1.position += multiplier * vec * distance
            if not node2.static: node2.position -= multiplier * vec * distance