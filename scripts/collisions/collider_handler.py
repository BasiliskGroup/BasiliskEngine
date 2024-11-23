import glm
import numpy as np
from scripts.collisions.collider import Collider
from scripts.collisions.broad.bounding_volume_heirarchy import BoundingVolumeHeirarchy
from scripts.collisions.narrow.narrow_collisions import get_narrow_collision
from scripts.physics.impulse import calculate_collisions
from scripts.transform_handler import TransformHandler

class ColliderHandler():
    def __init__(self, scene) -> None:
        self.scene = scene
        # vbos dictionary
        self.vbos = self.scene.vao_handler.vbo_handler.vbos
        self.colliders = []
        # transforms
        self.transform_handler = TransformHandler(self.scene)
        self.to_update = set([])
        
    def add(self, position:glm.vec3=None, scale:glm.vec3=None, rotation:glm.vec3=None, vbo:str='cube', static = True, elasticity:float=0.1, kinetic_friction:float=0.4, static_friction:float=0.8, group:str=None):
        """adds new collider with corresponding object"""
        self.colliders.append(Collider(self, position, scale, rotation, vbo, static, elasticity, kinetic_friction, static_friction, group))
        return self.colliders[-1]
    
    def resolve_collisions(self):
        """
        Resolves all collisions between colliders
        """
        for collider in self.colliders: 
            collider.has_collided      = False
            collider.has_hard_collided = False
            collider.collision_normals = {}
        
        # get broad collisions
        collider_vertices, needs_narrow = self.resolve_broad_collisions()
        
        # ensure that collisions are only checked once per pair
        for collider1, collider_list in needs_narrow.items():
            to_remove = set([])
            for collider2 in collider_list:
                if collider1.group is not None and collider2.group is not None and collider1.group == collider2.group: to_remove.add(collider2) 
                if collider2 in needs_narrow and collider1 in needs_narrow[collider2]: needs_narrow[collider2].remove(collider1)
            for collider in to_remove:
                collider_list.remove(collider)
                
        # narrow collisions
        self.resolve_narrow_collisions(collider_vertices, needs_narrow)
           
    def resolve_narrow_collisions(self, collider_vertices, needs_narrow) -> dict:
        for collider1, possible_colliders in needs_narrow.items():
            for collider2 in possible_colliders:
                
                node1 = collider1.node # TODO add support for colliders without nodes
                node2 = collider2.node
                
                #if collider1.static and collider2.static: continue
                if not (node1.physics_body or node2.physics_body): continue
                
                # check if already collided
                normal, distance, contact_points = get_narrow_collision(collider_vertices[collider1], collider_vertices[collider2], collider1.position, collider2.position)
                if abs(distance) < 1e-6: continue # continue if no collision
                
                # immediately resolve penetration
                collider1.has_collided = True
                collider2.has_collided = True
                
                # rel_vel = (node1.physics_body.velocity if node1.physics_body else glm.vec3(0)) - (node2.physics_body.velocity if node2.physics_body else glm.vec3(0))
                # n_rel_vel = abs(glm.dot(rel_vel, normal))
                # if n_rel_vel > 3: 
                #     collider1.has_hard_collided = n_rel_vel
                #     collider2.has_hard_collided = n_rel_vel
                
                if node1.physics_body: prev_vel1 = glm.vec3(node1.physics_body.velocity)
                if node2.physics_body: prev_vel2 = glm.vec3(node2.physics_body.velocity)
                
                if collider1.static: 
                    node2.position += normal * distance
                    collider2.collision_normals[node1] = normal # TODO may need to switch node with collider
                else:
                    if collider2.static: 
                        node1.position += normal * -distance
                        collider1.collision_normals[node2] = -normal
                    else:
                        node1.position += normal * 0.5 * -distance
                        node2.position += normal * 0.5 * distance
                        collider1.collision_normals[node2] = -normal
                        collider2.collision_normals[node1] = normal
                        
                #for both physics bodies
                if not (node1.physics_body or node2.physics_body): continue
                calculate_collisions(normal, collider1, collider2, node1.physics_body, node2.physics_body, contact_points, node1.get_inverse_inertia(), node2.get_inverse_inertia(), node1.geometric_center, node2.geometric_center)
                
                if node1.physics_body and glm.length(prev_vel1 - node1.physics_body.velocity) > 5: collider1.has_hard_collided = True
                if node2.physics_body and glm.length(prev_vel2 - node2.physics_body.velocity) > 5: collider2.has_hard_collided = True
                
    def resolve_broad_collisions(self) -> dict:
        """
        Resolves all collisions between colliders
        """
        # get broad collisions
        self.bvh.build_tree() # TODO replace with better update system
        
        needs_narrow = {}
        for collider in self.colliders:
            broad_collisions = self.bvh.get_collided(collider)
            if len(broad_collisions) < 1: continue
            needs_narrow[collider] = set(broad_collisions)
        
        # updates vertcies
        self.to_update.intersection_update(needs_narrow.keys())
        return self.update_vertices_transform(), needs_narrow

    def construct_bvh(self):
        """
        Creates the bvh for the first time. 
        """
        self.bvh = BoundingVolumeHeirarchy(self)
        
    def update_vertices_transform(self) -> dict:
        """
        Updates all necessary vertices using the transform handler
        """
            
        batch_data = []
        vert_rows = []
        
        for collider in self.to_update:
            # stack vertcies with center and dimensions
            vertex_data = np.copy(collider.unique_points)
            rows = vertex_data.shape[0]
            
            # adds transformation to point
            model_data = np.array([*collider.position, *collider.rotation, *collider.scale])
            collider_data = np.zeros(shape=(rows, 12), dtype="f4")
            
            collider_data[:,:3] = vertex_data
            collider_data[:,3:] = model_data
            
            batch_data.append(collider_data)
            vert_rows.append(rows)
        
        # Combine all meshes into a single array
        if len(batch_data) < 1:
            self.to_update = set([])
            return {} # if there are no broad collisions detected
            
        batch_data = np.vstack(batch_data, dtype="f4")
        data = self.transform_handler.transform('model_transform', batch_data)
        
        collider_data = {}
        
        for collider, count in zip(self.to_update, vert_rows):
            collider_data[collider] = []
            # get vertices
            #vertices = []
            for _ in range(count):
                collider_data[collider].append(glm.vec3(data[:3]))
                data = data[3:]
            #collider.vertices = vertices
        
        self.to_update = set([])
        return collider_data
    
    def remove(self, collider:Collider):
        if collider in self.colliders: self.colliders.remove(collider)
        del collider