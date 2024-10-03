import glm
import pygame as pg
from scripts.camera import *
from scripts.collisions.collider import Collider
from scripts.generic.data_types import vec3
from scripts.generic.math_functions import get_model_matrix, get_rotation_matrix
    
class Node():
    def __init__(self, node_handler, position:glm.vec3|list=None, scale:glm.vec3|list=None, rotation:glm.vec3|list=None, nodes:list=None, model=None, collider=None, physics_body=None, name:str='node', camera=None):
        # handler
        self.node_handler = node_handler
        
        # other
        self.name = name
        self.camera = camera
        
        # transforms
        self.position     = glm.vec3(position) if position else glm.vec3(0.0)
        self.scale        = glm.vec3(scale)    if scale    else glm.vec3(1.0)
        self.rotation     = glm.vec3(rotation) if rotation else glm.vec3(0.0)
        self.model_matrix = get_model_matrix(self.position, self.scale, self.rotation)
        
        # updating data
        self.prev_position = glm.vec3(self.position) 
        self.prev_scale    = glm.vec3(self.scale) # TODO check live scaling for nodes
        self.prev_rotation = glm.vec3(self.rotation)
        
        self.update_position = True
        self.update_scale    = True
        self.update_rotation = True
        
        self.update_inertia = True
        
        # children
        self.nodes:list[Node] = nodes if nodes else []
        self.model            = model
        self.collider         = collider
        self.physics_body     = physics_body
        
        # align children
        self.major_sync_data()
        self.define_geometric_center()
        
        # after children sync init
        self.aligned_inertia = self.define_inverse_inertia()
        self.inverse_inertia = self.get_inverse_inertia()
        
        # scripting
        self.on_tick      = None
        self.on_touch     = None
        self.on_collision = None # like touch but will have threashold
    
    # initialization
    def init_physics_body(self): 
        """
        Gives this node a physics body if it already has one and removes all bodies from children. 
        """
        temp_physics_body = self.physics_body # holds physics body in temp variable to save it after physics body destruction
        
        self.remove_physics_bodies()
        
        # reinitialize self physics body
        self.physics_body = temp_physics_body
        if self.physics_body: self.physics_body.rotation = glm.quat(self.rotation)
        
    def remove_physics_bodies(self):
        """
        Removes all the physics bodies from its children and itself.
        """
        for node in self.nodes: node.remove_physics_bodies()
        self.physics_body = None
    
    # updating
    def update(self, delta_time:float, ticked:bool=False):
        """
        Updates the position of this node based on the it's phsyics body and syncromizes all this children to its new data. 
        """
        self.before_update()
        
        self.define_inverse_inertia() # TODO remove line for debugging
            
        # update physics body
        if self.physics_body:
            delta_position = self.physics_body.get_delta_position(delta_time)
            self.position += delta_position
            self.rotation  = self.physics_body.get_new_rotation(delta_time)
            
        # update variables from last movement
        self.sync_data() 
        self.geometric_center = glm.vec3([*glm.mul(self.model_matrix, (*self.geometric_offset, 1))][:3]) # TODO get correct udpate with rotation
        
        if ticked and self.on_tick: exec(self.on_tick)
        
        self.after_update()
            
    def sync_data(self, position:glm.vec3=None, scale:glm.vec3=None, rotation:glm.vec3=None):
        """
        Synchronizes the position, scale, and rotation of all child nodes. Data may be inherited by a parent. 
        """
        if self.position.y < self.node_handler.death_plane: 
            self.position = glm.vec3(5, 5, 5)
            if self.physics_body: 
                self.physics_body.velocity = glm.vec3(0, 0, 0)
                self.physics_body.rotational_velocity = 0
                
        if position or scale or rotation:
        
            # attribute children
            child = glm.quat(self.rotation)
            parent = glm.quat(rotation)
            quat = child * parent
            rotation = glm.eulerAngles(quat)
            
            self.model_matrix = get_model_matrix(position, self.scale * scale, rotation)
            
            if self.model: 
                self.model.position = vec3(position)
                self.model.scale    = vec3(self.scale * scale)
                self.model.rotation = vec3(rotation)
                
            if self.collider: 
                self.collider.position = position
                self.collider.scale    = self.scale * scale
                self.collider.rotation = rotation
                
            if isinstance(self.camera, FollowCamera):
                self.camera.position = position
        
            # child nodes
            for node in self.nodes: 
                new_position = glm.mul(self.model_matrix, (*node.position, 1))
                node.sync_data(glm.vec3([*new_position][:3]), self.scale * scale, rotation) # TODO check if this works
            
        else: 
            
            self.model_matrix = get_model_matrix(self.position, self.scale, self.rotation)
            
            # attribute children
            if self.model: 
                self.model.position = vec3(self.position)
                self.model.scale    = vec3(self.scale)
                self.model.rotation = vec3(self.rotation)
                
            if self.collider: 
                self.collider.position = self.position
                self.collider.scale    = self.scale
                self.collider.rotation = self.rotation
                
            if self.camera:
                self.camera.position = self.position
            
            # child nodes
            for node in self.nodes: 
                new_position = glm.mul(self.model_matrix, (*node.position, 1))
                node.sync_data(glm.vec3([*new_position][:3]), self.scale, self.rotation)
                
        # update collider data if necessary
        if self.collider:   
            
            if self.update_scale or self.update_rotation:                         
                self.collider.update_dimensions()
                
            if self.update_position: 
                self.collider.update_geometric_center()
                
            self.collider.update_aabb()
            self.collider.collider_handler.to_update.add(self.collider)
            
    def major_sync_data(self):
        
        self.sync_data()
        self.define_geometric_center()
        
        # after children sync init
        self.aligned_inertia = self.define_inverse_inertia()
        self.inverse_inertia = self.get_inverse_inertia()
    
    # getter methods 
    def get_colliders(self):
        """
        Returns all colliders from this node's child nodes. 
        """
        colliders = [self.collider] if self.collider else []
        for node in self.nodes: colliders += node.get_colliders()
        return colliders
    
    def get_models(self):
        """
        Returns all models from this node's child nodes. 
        """
        models = [self.model] if self.model else []
        for node in self.nodes: models += node.get_models()
        return models
    
    def get_models_with_path(self):
        """
        Gets the models and their names from this node's children. 
        """
        names   = []
        models = []
        for node in self.nodes: 
            name, model = node.get_models_with_path()
            names      += name
            models     += model
        return [f'{self.name}>{name}' for name in names], models
    
    def add_node(self, node): 
        """
        Adds a node to this node's list. 
        """
        self.nodes.append(node)
    
    # physics function
    def define_inverse_inertia(self, density=0) -> glm.mat3x3:
        """
        Defines the inertia tensor for the node if the node has a collider
        """
        # determine the density of the node
        if density == 0: density = self.physics_body.mass / self.get_volume() if self.physics_body else 1
        mass = self.physics_body.mass if self.physics_body else 1
        
        x = 2 * self.scale.x
        y = 2 * self.scale.y
        z = 2 * self.scale.z
        
        return glm.inverse(mass / 12 * glm.mat3x3(y ** 2 + z ** 2, 0, 0, 0, x**2 + z**2, 0, 0, 0, x**2 + y**2))
            
        # # define current level inertia tensor
        # inertia_tensor = glm.mat3x3(0.0)
        
        # if self.collider: 
        #     # computes CONVEX inertia tensor
        #     for p in self.collider.unique_points: 
                
        #         x = p[0] * self.collider.scale.x
        #         y = p[1] * self.collider.scale.y
        #         z = p[2] * self.collider.scale.z
                
        #         inertia_tensor[0][0] += y * y + z * z
        #         inertia_tensor[1][1] += x * x + z * z
        #         inertia_tensor[2][2] += x * x + y * y
        #         inertia_tensor[0][1] -= x * y
        #         inertia_tensor[0][2] -= x * z
        #         inertia_tensor[1][2] -= y * z
                
        #     inertia_tensor[1][0] = inertia_tensor[0][1]
        #     inertia_tensor[2][0] = inertia_tensor[0][2]
        #     inertia_tensor[2][1] = inertia_tensor[1][2]
            
        #     inertia_tensor *= density * self.collider.get_volume() / len(self.collider.unique_points)
            
        # elif self.model:
            
        #     unique_points = self.model._Model__handler.vbos[self.model.vbo].unique_points
        #     for p in unique_points: 
                
        #         x = p[0] * self.model.scale.x
        #         y = p[1] * self.model.scale.y
        #         z = p[2] * self.model.scale.z
                
        #         inertia_tensor[0][0] += y * y + z * z
        #         inertia_tensor[1][1] += x * x + z * z
        #         inertia_tensor[2][2] += x * x + y * y
        #         inertia_tensor[0][1] -= x * y
        #         inertia_tensor[0][2] -= x * z
        #         inertia_tensor[1][2] -= y * z
                
        #     inertia_tensor[1][0] = inertia_tensor[0][1]
        #     inertia_tensor[2][0] = inertia_tensor[0][2]
        #     inertia_tensor[2][1] = inertia_tensor[1][2]
            
        #     inertia_tensor *= density * self.model.get_volume() / len(unique_points)
            
        # # define the inertia of all children
        # for node in self.nodes: node.define_inverse_inertia(density)
        
        # # sum child inertia tensors
        # for node in self.nodes:
        #     inverse_child_inertia = node.get_inverse_inertia()
        #     displacement = node.position
        #     if not inverse_child_inertia: continue
        #     child_inertia   = glm.inverse(inverse_child_inertia)
        #     inertia_tensor += child_inertia + (glm.dot(displacement, displacement) * glm.mat3x3() - glm.outerProduct(displacement, displacement))
            
        # if inertia_tensor == glm.mat3x3(0.0): return 
            
        return glm.inverse(inertia_tensor)
    
    def get_inverse_inertia(self):
        """
        Returns the inverse inertia tensor with the proper scaling and rotations
        """
        if not self.aligned_inertia: return None
        
        # gets the new inverse inertia if rotation has been changed. 
        if self.update_inertia: 
            rotation_matrix      = get_rotation_matrix(self.rotation) 
            self.inverse_inertia = rotation_matrix * self.aligned_inertia * glm.transpose(rotation_matrix) * (1/self.physics_body.mass if self.physics_body else 1)
            self.update_inertia  = False
            
        # return transformed inertia tensor
        return self.inverse_inertia
    
    def update_parents(self, parent=None):
        """
        Updates the parents of the children nodes. If no parent is passed then we assume this is the parent. 
        """
        if parent:
            for node in self.nodes: node.update_parents(parent)
            if self.collider: self.collider.node = parent
        else: 
            for node in self.nodes: node.update_parents(self)
            if self.collider: self.collider.node = self
            
    def before_update(self):
        """
        Updates previous data if data exceeds error value
        """
        # position
        if self.update_position and self.is_same_vec(self.prev_position, self.position): 
            self.position        = glm.vec3(self.prev_position)
            self.update_position = False
        else: self.prev_position = glm.vec3(self.position)
        
        # scale
        if self.update_scale and self.is_same_vec(self.prev_scale, self.scale):
            self.scale        = glm.vec3(self.prev_scale)
            self.update_scale = False
        else: self.prev_scale = glm.vec3(self.scale)
        
        # rotation
        if self.update_rotation and self.is_same_vec(self.prev_rotation, self.rotation):
            self.rotation        = glm.vec3(self.prev_rotation)
            self.update_rotation = False
            self.update_inertia  = False
        else: self.prev_rotation = glm.vec3(self.rotation)
        
    def after_update(self):
        """
        Called after all updates have been completed for singles and nodes
        """
        self.update_position = False
        self.update_rotation = False
        self.update_scale    = False
        
    def is_same_vec(self, vec1:list|glm.vec3, vec2:list|glm.vec3, epsilon:float=1e-7) -> bool:
        """
        Determines whether or not two vectors are far enough apart to be considered "similar".
        """
        return all(abs(v1 - v2) <= epsilon for v1, v2 in zip(vec1, vec2))
    
    def get_volume(self):
        """
        Gets the volume of the entire node heirarchy
        """
        volume = 0
        if self.collider:       volume += self.collider.get_volume()
        elif self.model:        volume += self.model.get_volume()
        for node in self.nodes: volume += node.get_volume()
        return volume
    
    def define_geometric_center(self):
        """
        Gets the geomatric center of the entire collection
        """
        # get child points and everage them
        top_right, bottom_left = self.get_aabb_points()
        self.geometric_center  = (top_right + bottom_left) / 2
        self.geometric_offset  = self.geometric_center - self.position
        
    def get_aabb_points(self):
        """
        Gets the top right and bottom left points of the collection's meshes. Returns the points and sets them
        """
        # determines current level points
        top_right, bottom_left = glm.vec3(-1e6), glm.vec3(1e6)
        if self.collider: top_right, bottom_left = self.collider.top_right, self.collider.bottom_left
        
        # gets the aabb of the children
        for node in self.nodes: 
            node_top_right, node_bottom_left = node.get_aabb_points()
            top_right, bottom_left = glm.max(top_right, node_top_right), glm.min(bottom_left, node_bottom_left)
        
        return top_right, bottom_left
    
    def apply_offset_force(self, force:glm.vec3, offset:glm.vec3, delta_time:float):
        """
        Applies the acceleration, translational and rotational, from an offset force
        """
        if not self.physics_body: return
        
        # translation
        self.physics_body.velocity += force / self.physics_body.mass * delta_time
        
        # rotation
        torque = glm.cross(offset, force)
        self.apply_torque(torque, delta_time)
            
    def apply_torque(self, torque:glm.vec3, delta_time:float):
        """
        Applies rotational acceleration from torque
        """
        if not self.physics_body: return
        
        # compute change in rotational velocity
        omega  = self.physics_body.rotational_velocity * self.physics_body.axis_of_rotation
        omega += self.inverse_inertia * torque * delta_time
        
        if (rotational_velocity := glm.length(omega)) < 1e-7:
            self.physics_body.rotational_velocity = 0
        else:
            self.physics_body.rotational_velocity = rotational_velocity
            self.physics_body.axis_of_rotation    = glm.normalize(omega)
            
    # position
    @property
    def position(self): 
        return self._position
    
    @position.setter
    def position(self, value):
        self.update_position = True
        self._position       = value
    
    # scale
    @property
    def scale(self): 
        return self._scale
    
    @scale.setter
    def scale(self, value):
        self.update_scale = True
        self._scale       = value
        
    # rotation
    @property
    def rotation(self): return self._rotation
    
    @rotation.setter
    def rotation(self, value):
        self.update_rotation        = True
        self.update_rotation_matrix = True
        if isinstance(value, glm.quat) and self.physics_body: self.physics_body.rotation = value
        self._rotation              = value
        
    # nodes
    @property
    def collider(self): return self._collider
    
    @collider.setter
    def collider(self, value):
        if isinstance(value, Collider): value.node = self
        self._collider = value