import glm
import numpy as np
from .helper import node_is
from ..generic.vec3 import Vec3
from ..generic.quat import Quat
from ..generic.matrices import get_model_matrix
from ..mesh.mesh import Mesh
from ..render.material import Material
from ..physics.physics_body import PhysicsBody
from ..collisions.collider import Collider
from ..render.chunk import Chunk
from ..render.shader import Shader


class Node():
    position: Vec3
    """The position of the node in meters with swizzle xyz"""
    scale: Vec3
    """The scale of the node in meters in each direction"""
    rotation: Quat
    """The rotation of the node"""
    relative_position: bool
    """The position of this node relative to the parent node"""
    relative_scale: bool
    """The scale of this node relative to the parent node"""
    relative_rotation: bool
    """The rotation of this node relative to the parent node"""
    forward: glm.vec3
    """The forward facing vector of the node"""
    mesh: Mesh
    """The mesh of the node stored as a basilisk mesh object"""
    material: Material
    """The mesh of the node stored as a basilisk material object"""
    velocity: glm.vec3
    """The translational velocity of the node"""
    rotational_velocity: glm.vec3
    """The rotational velocity of the node"""
    physics: bool
    """Allows the node's movement to be affected by the physics engine and collisions"""
    mass: float
    """The mass of the node in kg"""
    collision: bool
    """Gives the node collision with other nodes in the scene""" 
    collider_mesh: str
    """The collider type of the node. Can be either 'box' or 'mesh'"""
    static_friction: float
    """Determines the friction of the node when still: recommended value 0.0 - 1.0"""
    kinetic_friction: float
    """Determines the friction of the node when moving: recommended value 0.0 - 1.0"""  
    elasticity: float
    """Determines how bouncy an object is: recommended value 0.0 - 1.0"""
    collision_group: str
    """Nodes of the same collision group do not collide with each other"""
    name: str
    """The name of the node for reference"""  
    tags: list[str]
    """Tags are used to sort nodes into separate groups"""
    static: bool
    """Objects that don't move should be marked as static"""
    chunk: Chunk
    """The parent chunk of the node. Used for callbacks to update chunk meshes"""
    children: list
    """List of nodes that this node is a parent of"""
    shader: Shader
    """Shader that is used to render the node. If none is given, engine default will be used"""

    def __init__(self,
            position:            glm.vec3=None, 
            scale:               glm.vec3=None, 
            rotation:            glm.quat=None, 
            relative_position:   bool=True,
            relative_scale:      bool=True,
            relative_rotation:   bool=True,
            forward:             glm.vec3=None, 
            mesh:                Mesh=None, 
            material:            Material=None, 
            velocity:            glm.vec3=None, 
            rotational_velocity: glm.vec3=None, 
            physics:             bool=False, 
            mass:                float=None, 
            collision:           bool=False, 
            collider_mesh:       str|Mesh=None, 
            static_friction:     float=None, 
            kinetic_friction:    float=None, 
            elasticity:          float=None, 
            collision_group:     float=None, 
            name:                str='', 
            tags:                list[str]=None,
            static:              bool=None,
            shader:              Shader=None
        ) -> None:
        """
        Basilisk node object. 
        Contains mesh data, translation, material, physics, collider, and descriptive information. 
        Base building block for populating a Basilisk scene.
        """
        
        # parents
        self.node_handler = None
        self.scene        = None
        self.engine       = None
        self.chunk        = None
        self.parent       = None
        
        # lazy update variables
        self.needs_geometric_center = True # pos
        self.needs_model_matrix = True # pos, scale, rot

        # node data
        self.internal_position: Vec3 = Vec3(position) if position else Vec3(0, 0, 0)
        self.internal_scale   : Vec3 = Vec3(scale)    if scale    else Vec3(1, 1, 1)
        self.internal_rotation: Quat = Quat(rotation) if rotation else Quat(1, 0, 0, 0)
        
        # relative transformations
        self.relative_position = glm.vec3(0, 0, 0)    if relative_position else None
        self.relative_scale    = glm.vec3(0, 0, 0)    if relative_scale    else None
        self.relative_rotation = glm.quat(1, 0, 0, 0) if relative_rotation else None
        
        self.forward  = forward  if forward  else glm.vec3(1, 0, 0)
        self.mesh     = mesh
        self._mtl_list = material if isinstance(material, list) else [material]
        self.material = material if material else None
        self.velocity = velocity if velocity else glm.vec3(0, 0, 0)
        self.rotational_velocity = rotational_velocity if rotational_velocity else glm.vec3(0, 0, 0)
        
        self._static = static

        # Physics updates
        if physics: self.physics_body = PhysicsBody(mass = mass if mass else 1.0)
        elif mass: raise ValueError('Node: cannot have mass if it does not have physics')
        else: self.physics_body = None
        
        # collider
        if collision: 
            self.collider = Collider(
                node = self,
                collider_mesh = collider_mesh,
                static_friction = static_friction,
                kinetic_friction = kinetic_friction,
                elasticity = elasticity,
                collision_group = collision_group
            )
        elif collider_mesh:         raise ValueError('Node: cannot have collider mesh if it does not allow collisions')
        elif static_friction:  raise ValueError('Node: cannot have static friction if it does not allow collisions')
        elif kinetic_friction: raise ValueError('Node: cannot have kinetic friction if it does not allow collisions')
        elif elasticity:       raise ValueError('Node: cannot have elasticity if it does not allow collisions')
        elif collision_group:  raise ValueError('Node: cannot have collider group if it does not allow collisions')
        else: self.collider = None

        # information and recursion
        self.name = name
        self.tags = tags if tags else []

        self.data_index = 0
        self.children = []

        # Shader given by user or none for default
        self.shader = shader

        # callback function to be added to the custom Vec3 and Quat classes
        def position_callback():
            if self.chunk:
                self.chunk.node_update_callback(self)
            
            # update variables
            self.needs_geometric_center = True
            self.needs_model_matrix = True
            if self.collider:
                self.collider.needs_bvh = True
                self.collider.needs_obb = True
            
        def scale_callback():
            if self.chunk:
                self.chunk.node_update_callback(self)
            
            # update variables
            self.needs_model_matrix = True
            if self.collider:
                self.collider.needs_bvh = True
                self.collider.needs_obb = True
                self.collider.needs_half_dimensions = True
            
        def rotation_callback():
            if self.chunk:
                self.chunk.node_update_callback(self)
            
            # update variables
            self.needs_model_matrix = True
            if self.collider:
                self.collider.needs_bvh = True
                self.collider.needs_obb = True
                self.collider.needs_half_dimensions = True
        
        self.internal_position.callback = position_callback
        self.internal_scale.callback    = scale_callback
        self.internal_rotation.callback = rotation_callback
    
    def init_scene(self, scene: ...) -> None:
        """
        Updates the scene of the node
        """
        self.scene = scene
        self.engine = scene.engine
        self.node_handler = scene.node_handler

        # Update materials
        self.write_materials()

        # Update the mesh
        self.mesh = self.mesh if self.mesh else self.engine.cube

        # Update physics and collider
        if self.physics_body: self.physics_body.physics_engine = scene.physics_engine
        if self.collider: self.collider.collider_handler = scene.collider_handler

    def update(self, dt: float) -> None:
        """
        Updates the node's movement variables based on the delta time
        """
        # update based on physical properties
        if any(self.velocity): self.position += dt * self.velocity
        if any(self.rotational_velocity): self.rotation = glm.normalize(self.rotation.data - dt / 2 * self.rotation.data * glm.quat(0, *self.rotational_velocity))

        if self.physics_body:
            self.velocity += self.physics_body.get_delta_velocity(dt)
            self.rotational_velocity += self.physics_body.get_delta_rotational_velocity(dt)
            
        # update children transforms
        for child in self.children: child.sync_data()
        
    def sync_data(self) -> None:
        """
        Syncronizes this node with the parent node based on its relative positioning
        """
        # calculate transform matrix with the given input
        transform = glm.mat4x4()
        if self.relative_position: transform  = glm.translate(transform, self.parent.position.data)
        if self.relative_rotation: transform *= glm.transpose(glm.mat4_cast(self.parent.rotation.data))
        if self.relative_scale:    transform  = glm.scale(transform, self.parent.scale.data)
        
        # set this node's transforms based on the parent
        if self.relative_position: self.position = transform * self.relative_position
        if self.relative_scale:    self.scale = self.relative_scale * self.parent.scale.data
        if self.relative_rotation: self.rotation = self.relative_rotation * self.parent.rotation.data
        
        for child in self.children: child.sync_data()
        
    def deep_copy(self) -> ...:
        """
        Creates a deep copy of this node and returns it. The new node is not added to the scene.
        """
        
        copy = Node(
            position = self.position,
            scale = self.scale,
            rotation = self.rotation,
            relative_position = bool(self.relative_position),
            relative_scale = bool(self.relative_scale),
            relative_rotation = bool(self.relative_rotation),
            forward = glm.vec3(self.forward),
            mesh = self.mesh,
            material = self.material,
            velocity = glm.vec3(self.velocity),
            rotational_velocity = glm.vec3(self.rotational_velocity),
            physics = bool(self.physics_body),
            mass = self.mass if self.physics_body else None,
            collision = bool(self.collider),
            static_friction = self.static_friction if self.collider else None,
            kinetic_friction = self.kinetic_friction if self.collider else None,
            elasticity = self.elasticity if self.collider else None,
            collision_group = self.collision_group if self.collider else None,
            name = self.name,
            tags = [tag for tag in self.tags], # deep copy tags list
            static = self.static,
            shader = self.shader
        )
        
        return copy
    
    def get_all(self, position: glm.vec3=None, scale: glm.vec3=None, rotation: glm.quat=None, forward: glm.vec3=None, mesh: Mesh=None, material: Material=None, velocity: glm.vec3=None, rotational_velocity: glm.quat=None, physics: bool=None, mass: float=None, collisions: bool=None, static_friction: float=None, kinetic_friction: float=None, elasticity: float=None, collision_group: float=None, name: str=None, tags: list[str]=None,static: bool=None) -> list:
        nodes = [self] if node_is(self, position, scale, rotation, forward, mesh, material, velocity, rotational_velocity, physics, mass, collisions, static_friction, kinetic_friction, elasticity, collision_group, name, tags, static) else []
        for node in self.children: nodes += node.get_all(position, scale, rotation, forward, mesh, material, velocity, rotational_velocity, physics, mass, collisions, static_friction, kinetic_friction, elasticity, collision_group, name, tags, static)
        return nodes
        
    # tree functions for managing children 
    def add(self, child: ..., relative_position: bool=None, relative_scale: bool=None, relative_rotation: glm.vec3=None) -> None:
        """
        Adopts a node as a child. Relative transforms can be changed, if left bank they will not be chnaged from the current child nodes settings.
        """
        if child in self.children or child is self: return
        assert isinstance(child, Node), 'Nodes can only accept other Nodes as children.'
        
        relative = glm.inverse(self.model_matrix) * child.model_matrix
        position = glm.vec3(relative[3])
        scale = glm.vec3([glm.length(relative[i]) for i in range(3)])
        rotation = glm.quat_cast(glm.mat3x3(*[glm.vec3(relative[i]) / scale[i] for i in range(3)])) # TODO ensure that these are the correct calculations
        
        # compute relative transformations
        if relative_position or (relative_position is None and child.relative_position): child.relative_position = position
        if relative_scale    or (relative_scale    is None and child.relative_scale):    child.relative_scale    = scale
        if relative_rotation or (relative_rotation is None and child.relative_rotation): child.relative_rotation = rotation
        
        # add as a child to by synchronized and controlled
        if self.node_handler: self.node_handler.add(child)
        child.parent = self
        self.children.append(child)
        
    def remove(self, child: ...) -> None:
        """
        Removes a child node from this nodes chlid list.
        """
        if child in self.children: 
            if self.node_handler: self.node_handler.remove(child)
            child.parent = None
            self.children.remove(child)
        
    def apply_force(self, force: glm.vec3, dt: float) -> None:
        """
        Applies a force at the center of the node
        """
        self.apply_offset_force(force, glm.vec3(0.0), dt)
        
    def apply_offset_force(self, force: glm.vec3, offset: glm.vec3, dt: float) -> None:
        """
        Applies a force at the given offset
        """
        # translation
        assert self.physics_body, 'Node: Cannot apply a force to a node that doesn\'t have a physics body'
        self.velocity += force / self.mass * dt
        
        # rotation
        torque = glm.cross(offset, force)
        self.apply_torque(torque, dt)
        
    def apply_torque(self, torque: glm.vec3, dt: float) -> None:
        """
        Applies a torque on the node
        """
        assert self.physics_body, 'Node: Cannot apply a torque to a node that doesn\'t have a physics body'
        ...
    
    # TODO change geometric variables into properties
    def get_inverse_inertia(self) -> glm.mat3x3:
        """
        Transforms the mesh inertia tensor and inverts it
        """
        if not ((self.mesh or (self.collider and self.collider.mesh)) and self.physics_body): return None 
        mesh = self.collider.mesh if self.collider else self.mesh
        inertia_tensor = mesh.get_inertia_tensor(self.scale) / 2
    
        # mass
        if self.physics_body: inertia_tensor *= self.physics_body.mass
                
        # rotation
        rotation_matrix = glm.mat3_cast(self.rotation.data)
        inertia_tensor  = rotation_matrix * inertia_tensor * glm.transpose(rotation_matrix)
        
        return glm.inverse(inertia_tensor)
    
    def get_vertex(self, index) -> glm.vec3:
        """
        Gets the world space position of a vertex indicated by the index in the mesh
        """
        return glm.vec3(self.model_matrix * glm.vec4(*self.mesh.points[index], 1))

    def get_data(self) -> np.ndarray:
        """
        Gets the node batch data for chunk batching
        """
        
        # Get data from the mesh node
        mesh_data = self.mesh.data
        node_data = np.array([*self.position, *self.rotation, *self.scale, 0])

        per_vertex_mtl = isinstance(self.material, list)

        if not per_vertex_mtl: node_data[-1] = self.material.index

        # Create an array to hold the node's data
        width = 25 if not self.mesh.custom else 11 + mesh_data.shape[1]
        data = np.zeros(shape=(mesh_data.shape[0], width), dtype='f4')


        data[:,:mesh_data.shape[1]] = mesh_data
        data[:,mesh_data.shape[1]:] = node_data

        if per_vertex_mtl: data[:,-1] = self.material

        if self.shader and not self.mesh.custom: data = np.take(data, self.shader.attribute_indices, axis=1)

        return data

    def write_materials(self):
        """
        Internal function to write the material list to the material handler and get the material ids
        """

        if isinstance(self.material, list):
            mtl_index_list = []
            for mtl in self._mtl_list:
                self.engine.material_handler.add(mtl)
                mtl_index_list.append(mtl.index)
                mtl_index_list.append(mtl.index)
                mtl_index_list.append(mtl.index)
            self._material = mtl_index_list

        if isinstance(self.material, type(None)):
            self.material = self.engine.material_handler.base
        

    def __repr__(self) -> str:
        """
        Returns a string representation of the node
        """

        return f'<Bailisk Node | {self.name}, {self.mesh}, ({self.position})>'
    
    @property
    def position(self): return self.internal_position
    @property
    def scale(self):    return self.internal_scale
    @property
    def rotation(self): return self.internal_rotation
    @property
    def forward(self):  return self._forward
    @property
    def mesh(self):     return self._mesh
    @property
    def material(self): return self._material
    @property
    def velocity(self): return self._velocity
    @property
    def rotational_velocity(self): return self._rotational_velocity
    @property
    def mass(self): 
        if self.physics_body: return self.physics_body.mass
        raise RuntimeError('Node: Cannot access the mass of a node that has no physics body')
    @property
    def static_friction(self):
        if self.collider: return self.collider.static_friction
        raise RuntimeError('Node: Cannot access the static friction of a node that has no collider')
    @property
    def kinetic_friction(self):
        if self.collider: return self.collider.kinetic_friction
        raise RuntimeError('Node: Cannot access the kinetic friction of a node that has no collider')
    @property
    def elasticity(self):
        if self.collider: return self.collider.elasticity
        raise RuntimeError('Node: Cannot access the elasticity of a node that has no collider')
    @property
    def collision_group(self):
        if self.collider: return self.collider.collision_group
        raise RuntimeError('Node: Cannot access the collision_group of a node that has no collider')
    @property
    def name(self): return self._name
    @property
    def tags(self): return self._tags
    @property
    def static(self):
        return self._static if self._static is not None else not(self.physics or any(self.velocity) or any(self.rotational_velocity) or (self.parent and not self.parent.static))
    @property
    def x(self): return self.internal_position.data.x
    @property
    def y(self): return self.internal_position.data.y
    @property
    def z(self): return self.internal_position.data.z
    
    # TODO add descriptions in the class header
    @property
    def model_matrix(self): 
        if self.needs_model_matrix: 
            self._model_matrix = get_model_matrix(self.position, self.scale, self.rotation)
            self.needs_model_matrix = False
        return self._model_matrix
    @property
    def geometric_center(self): # assumes the node has a mesh
        # if not self.mesh: raise RuntimeError('Node: Cannot retrieve geometric center if node does not have mesh')
        if self.needs_geometric_center: 
            self._geometric_center = self.model_matrix * self.mesh.geometric_center
            self.needs_geometric_center = False
        return self._geometric_center
    @property
    def center_of_mass(self): 
        if not self.mesh: raise RuntimeError('Node: Cannot retrieve center of mass if node does not have mesh')
        return self.model_matrix * self.mesh.center_of_mass
    @property
    def volume(self):
        if not self.mesh: raise RuntimeError('Node: Cannot retrieve volume if node does not have mesh')
        return self.mesh.volume * self.scale.x * self.scale.y * self.scale.z
    
    @property
    def physics(self):
        return bool(self.physics_body)
    @property
    def collision(self):
        return bool(self.collider)
    
    @property
    def collisions(self):
        assert self.collision, 'Node: Cannot access collision data without collisions enabled on Node'
        return self.collider.collisions
    
    @position.setter
    def position(self, value: tuple | list | glm.vec3 | np.ndarray):
        if isinstance(value, glm.vec3): self.internal_position.data = value
        elif isinstance(value, Vec3): self.internal_position.data = value.data
        elif isinstance(value, tuple) or isinstance(value, list) or isinstance(value, np.ndarray):
            if len(value) != 3: raise ValueError(f'Node: Invalid number of values for position. Expected 3, got {len(value)}')
            self.internal_position.data = glm.vec3(value)
        else: raise TypeError(f'Node: Invalid position value type {type(value)}')
        
        # # compute relative position if all cases passed
        # if self.parent and self.relative_position: self.relative_position = self.internal_position.data - self.parent.position
    
    @scale.setter
    def scale(self, value: tuple | list | glm.vec3 | np.ndarray):
        if isinstance(value, glm.vec3): self.internal_scale.data = value
        elif isinstance(value, Vec3): self.internal_scale.data = value.data
        elif isinstance(value, tuple) or isinstance(value, list) or isinstance(value, np.ndarray):
            if len(value) != 3: raise ValueError(f'Node: Invalid number of values for scale. Expected 3, got {len(value)}')
            self.internal_scale.data = glm.vec3(value)
        else: raise TypeError(f'Node: Invalid scale value type {type(value)}')
        
        # # compute relative scale
        # if self.parent and self.relative_scale: self.relative_scale = self.internal_scale.data / self.parent.scale

    @rotation.setter
    def rotation(self, value: tuple | list | glm.vec3 | glm.quat | glm.vec4 | np.ndarray):
        if isinstance(value, glm.quat) or isinstance(value, glm.vec4) or isinstance(value, glm.vec3): self.internal_rotation.data = glm.quat(value)
        elif isinstance(value, Quat): self.internal_rotation.data = value.data
        elif isinstance(value, tuple) or isinstance(value, list) or isinstance(value, np.ndarray):
            if len(value) == 3: self.internal_rotation.data = glm.quat(glm.vec3(*value))
            elif len(value) == 4: self.internal_rotation.data = glm.quat(*value)
            else: raise ValueError(f'Node: Invalid number of values for rotation. Expected 3 or 4, got {len(value)}')
        else: raise TypeError(f'Node: Invalid rotation value type {type(value)}')
        
        # # compute relative rotation
        # if self.parent and self.relative_rotation: self.relative_rotation = self.rotation * glm.inverse(self.parent.rotation)

    @forward.setter
    def forward(self, value: tuple | list | glm.vec3 | np.ndarray):
        if isinstance(value, glm.vec3): self._forward = value
        elif isinstance(value, tuple) or isinstance(value, list) or isinstance(value, np.ndarray):
            if len(value) != 3: raise ValueError(f'Node: Invalid number of values for forward. Expected 3, got {len(value)}')
            self._forward = glm.vec3(value)
        else: raise TypeError(f'Node: Invalid forward value type {type(value)}')
        
    @mesh.setter
    def mesh(self, value: Mesh | None):
        if isinstance(value, Mesh):
            self._mesh = value
            if self.chunk: self.chunk.update()
        elif isinstance(value, type(None)):
            self._mesh = None
            if not self.chunk: return
            self.chunk.remove(self)
            self.chunk.update()
        else: raise TypeError(f'Node: Invalid mesh value type {type(value)}')
    
    @material.setter
    def material(self, value: Material):
        if isinstance(value, list):
            self._mtl_list = value
            if not self.node_handler: 
                self._material = value
            else:
                mtl_index_list = []
                for mtl in self._mtl_list:
                    self.engine.material_handler.add(mtl)
                    mtl_index_list.append(mtl.index)
                    mtl_index_list.append(mtl.index)
                    mtl_index_list.append(mtl.index)
                self._material = mtl_index_list
        elif isinstance(value, Material): 
            self._material = value
            if self.node_handler: self.engine.material_handler.add(value)
        elif isinstance(value, type(None)):
            if self.engine: self._material = self.engine.material_handler.base
            else: self._material = None

        else: raise TypeError(f'Node: Invalid material value type {type(value)}')
        if not self.chunk: return
        self.chunk.node_update_callback(self)
    
    @velocity.setter
    def velocity(self, value: tuple | list | glm.vec3 | np.ndarray | Vec3):
        if isinstance(value, glm.vec3): self._velocity = glm.vec3(value)
        elif isinstance(value, Vec3): self._velocity = glm.vec3(value.data)
        elif isinstance(value, tuple) or isinstance(value, list) or isinstance(value, np.ndarray):
            if len(value) != 3: raise ValueError(f'Node: Invalid number of values for velocity. Expected 3, got {len(value)}')
            self._velocity = glm.vec3(value)
        else: raise TypeError(f'Node: Invalid velocity value type {type(value)}')
        
    @rotational_velocity.setter
    def rotational_velocity(self, value: tuple | list | glm.vec3 | np.ndarray | Vec3):
        if isinstance(value, glm.vec3): self._rotational_velocity = glm.vec3(value)
        elif isinstance(value, Vec3): self._rotational_velocity = glm.vec3(value.data)
        elif isinstance(value, tuple) or isinstance(value, list) or isinstance(value, np.ndarray):
            if len(value) != 3: raise ValueError(f'Node: Invalid number of values for rotational velocity. Expected 3, got {len(value)}')
            self._rotational_velocity = glm.vec3(value)
        else: raise TypeError(f'Node: Invalid rotational velocity value type {type(value)}')
        
    @mass.setter
    def mass(self, value: int | float):
        if not self.physics_body: raise RuntimeError('Node: Cannot set the mass of a node that has no physics body')
        if isinstance(value, int) or isinstance(value, float): self.physics_body.mass = value
        else: raise TypeError(f'Node: Invalid mass value type {type(value)}')
        
    @static_friction.setter
    def static_friction(self, value: int | float):
        if not self.collider: raise RuntimeError('Node: Cannot set the static friction of a node that has no physics body')
        if isinstance(value, int) or isinstance(value, float): self.collider.static_friction = value
        else: raise TypeError(f'Node: Invalid static friction value type {type(value)}')
    
    @kinetic_friction.setter
    def kinetic_friction(self, value: int | float):
        if not self.collider: raise RuntimeError('Node: Cannot set the kinetic friction of a node that has no physics body')
        if isinstance(value, int) or isinstance(value, float): self.collider.kinetic_friction = value
        else: raise TypeError(f'Node: Invalid kinetic friction value type {type(value)}')
        
    @elasticity.setter
    def elasticity(self, value: int | float):
        if not self.collider: raise RuntimeError('Node: Cannot set the elasticity of a node that has no physics body')
        if isinstance(value, int) or isinstance(value, float): self.collider.elasticity = value
        else: raise TypeError(f'Node: Invalid elasticity value type {type(value)}')
        
    @collision_group.setter
    def collision_group(self, value: str):
        if not self.collider: raise RuntimeError('Node: Cannot set the collision gruop of a node that has no physics body')
        if isinstance(value, (str, type(None))): self.collider.collision_group = value
        else: raise TypeError(f'Node: Invalid collision group value type {type(value)}')
        
    @name.setter
    def name(self, value: str):
        if isinstance(value, str): self._name = value
        else: raise TypeError(f'Node: Invalid name value type {type(value)}')
        
    @tags.setter
    def tags(self, value: list[str]):
        if isinstance(value, list) or isinstance(value, tuple):
            for tag in value:
                if not isinstance(tag, str): raise TypeError(f'Node: Invalid tag value in tags list of type {type(tag)}')
            self._tags = value
        else: raise TypeError(f'Node: Invalid tags value type {type(value)}')
        
    @static.setter
    def static(self, value: bool):
        self._static = value

    @x.setter
    def x(self, value: int | float):
        if isinstance(value, int) or isinstance(value, float): self.internal_position.x = value
        else: raise TypeError(f'Node: Invalid positional x value type {type(value)}')
        
    @y.setter
    def y(self, value: int | float):
        if isinstance(value, int) or isinstance(value, float): self.internal_position.y = value
        else: raise TypeError(f'Node: Invalid positional y value type {type(value)}')
        
    @z.setter
    def z(self, value: int | float):
        if isinstance(value, int) or isinstance(value, float): self.internal_position.z = value
        else: raise TypeError(f'Node: Invalid positional z value type {type(value)}')
        
    @physics.setter
    def physics(self, value: bool | PhysicsBody):
        if not value and self.physics: # remove physics body from self and scene
            if self.node_handler: self.node_handler.scene.physics_engine.remove(self.physics_body)
            self.physics_body = None
        elif isinstance(value, PhysicsBody): # deep copy physics body
            if self.physics: 
                self.mass = value.mass
            else: 
                self.physics_body = PhysicsBody(value.mass)
                if self.node_handler: self.physics_body.physics_engine = self.node_handler.scene.physics_engine
        elif not self.physics:
            self.physics_body = PhysicsBody(mass = 1)
            if self.node_handler: self.physics_body.physics_engine = self.node_handler.scene.physics_engine

            
    @collision.setter
    def collision(self, value: bool | PhysicsBody):
        if not value and self.collision:
            if self.node_handler: self.node_handler.scene.collider_handler.remove(self.collider)
            self.collider = None
        elif isinstance(value, Collider):
            if self.collision:
                self.kinetic_friction = value.kinetic_friction
                self.elasticity = value.elasticity
                self.static_friction = value.static_friction
            else:
                self.collider = Collider(self, value.mesh, value.static_friction, value.kinetic_friction, value.elasticity, value.collision_group)
                if self.node_handler: self.collider.collider_handler = self.node_handler.scene.collider_handler
        elif not self.collider:
            self.collider = Collider(self)
            if self.node_handler: self.collider.collider_handler = self.node_handler.scene.collider_handler