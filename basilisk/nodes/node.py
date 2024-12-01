import glm
import numpy as np
from ..generic.vec3 import Vec3
from ..render.mesh import Mesh
from ..render.material import Material
from ..physics.physics_body import PhysicsBody
from ..collisions.collider import Collider
from ..render.chunk import Chunk

class Node():
    position: Vec3
    """The position of the node in meters with swizzle xyz"""
    scale: Vec3
    """The scale of the node in meters in each direction"""
    rotation: glm.quat # TODO add custom quat class
    """The rotation of the node"""
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
    collisions: bool
    """Gives the node collision with other nodes in the scene""" 
    collider: str
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
    """""" # TODO Jonah description
    children: list
    """List of nodes that this node is a parent of"""

    def __init__(self, node_handler,
            position:            Vec3=None, 
            scale:               Vec3=None, 
            rotation:            glm.quat=None, 
            forward:             glm.vec3=None, 
            mesh:                Mesh=None, 
            material:            Material=None, 
            velocity:            glm.vec3=None, 
            rotational_velocity: glm.vec3=None, 
            physics:             bool=False, 
            mass:                float=None, 
            collisions:          bool=False, 
            collider:            str=None, 
            static_friction:     float=None, 
            kinetic_friction:    float=None, 
            elasticity:          float=None, 
            collision_group :    float=None, 
            name:                str='', 
            tags:                list[str]=None,
            static:              bool=True
        ) -> None:
        """
        Basilisk node object. 
        Contains mesh data, translation, material, physics, collider, and descriptive information. 
        Base building block for populating a Basilisk scene.
        """
        
        self.node_handler = node_handler
        self.chunk = None

        self.internal_position: Vec3 = Vec3(position) if position else Vec3(0, 0, 0)
        self.internal_scale   : Vec3 = Vec3(scale)    if scale    else Vec3(1, 1, 1)
        self.rotation = rotation if rotation else glm.quat(1, 0, 0, 0)
        self.forward  = forward  if forward  else glm.vec3(1, 0, 0)
        self.mesh     = mesh     if mesh     else None # TODO add default cube mesh
        self.material = material if material else None # TODO add default base material
        self.velocity = velocity if velocity else glm.vec3(0, 0, 0)
        self.rotational_velocity = rotational_velocity if rotational_velocity else glm.vec3(0, 0, 0)
        
        if physics: self.physics_body: PhysicsBody = self.node_handler.scene.physics_engine.add(mass)
        elif mass: raise ValueError('Node cannot have mass if it does not have physics')
        else: self.physics_body = None
        
        if collisions: self.collider: Collider = ...
        elif collider:         raise ValueError('Node cannot have collider mesh if it does not allow collisions')
        elif static_friction:  raise ValueError('Node cannot have static friction if it does not allow collisions')
        elif kinetic_friction: raise ValueError('Node cannot have kinetic friction if it does not allow collisions')
        elif elasticity:       raise ValueError('Node cannot have elasticity if it does not allow collisions')
        elif collision_group:  raise ValueError('Node cannot have collider group if it does not allow collisions')
        else: self.collider = None
        
        self.name = name
        self.tags = tags if tags else []
        self.static = static and not (self.physics_body or self.velocity or self.rotational_velocity)
        self.children = []
        
    def update(self, dt: float) -> None:
        """
        Updates the node's movement variables based on the delta time
        """
        self.position += dt * self.velocity
        self.rotation += 0.5 * dt * self.rotation * glm.quat(0, *self.rotational_velocity)
        self.rotation = glm.normalize(self.rotation)
        
        if self.physics_body:
            self.velocity += self.physics_body.get_delta_velocity(dt)
            self.rotational_velocity += self.physics_body.get_delta_rotational_velocity(dt)
        
    def sync_data(self, dt: float) -> None: # TODO only needed for child nodes now
        ...
        
    def get_nodes(self, 
            require_mesh: bool=False, 
            require_collider: bool=False, 
            require_physics_body: bool=False, 
            filter_material: Material=None, 
            filter_tags: list[str]=None
        ) -> list: 
        """
        Returns the nodes matching the required filters from this branch of the nodes
        """
        # adds self to nodes list if it matches the criteria
        nodes = []
        if  (not require_mesh or self.mesh) \
        and (not require_collider or self.collider) \
        and (not require_physics_body or self.physics_body) \
        and (not filter_material or self.material == filter_material) \
        and (not filter_tags or all([tag in self.tags for tag in filter_tags])): 
            nodes.append(self)
        
        # adds children to nodes list if they match the criteria
        for node in self.children: nodes.extend(node.get_nodes(require_mesh, require_collider, require_physics_body, filter_material, filter_tags))
        return node 
        
    def adopt_child(self, node) -> None: # TODO determine the best way for the user to do this through the scene
        ...
        
    def add_child(self) -> None: # TODO add node constructor
        ... 
        
    def get_inverse_inertia(self) -> glm.mat3x3: # TODO add checks for collider and physics body
        ...
        
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
        self.velocity = force / self.mass * dt
        
        # rotation
        torque = glm.cross(offset, force)
        self.apply_torque(torque, dt)
        
    def apply_torque(self, torque: glm.vec3, dt: float) -> None:
        """
        Applies a torque on the node
        """
        assert self.physics_body, 'Node: Cannot apply a torque to a node that doesn\'t have a physics body'
        ...

    def __repr__(self) -> str:
        """
        Returns a string representation of the node
        """

        return f'<Bailisk Node | {self.name}, {self.mesh}, ({self.position})>'
    
    @property
    def position(self): return self.internal_position.data
    @property
    def scale(self):    return self.internal_scale.data
    @property
    def rotation(self): return self._rotation
    @property
    def forward(self):  return self._forward
    # TODO add property for Mesh
    # TODO add property for Material
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
    def x(self): return self.internal_position.data.x # TODO test these functions
    @property
    def y(self): return self.internal_position.data.y
    @property
    def z(self): return self.internal_position.data.z
    
    @position.setter
    def position(self, value: tuple | list | glm.vec3 | np.ndarray):
        if isinstance(value, glm.vec3): self.internal_position.data = glm.vec3(value)
        elif isinstance(value, tuple) or isinstance(value, list) or isinstance(value, np.ndarray):
            if len(value) != 3: raise ValueError(f'Node: Invalid number of values for position. Expected 3, got {len(value)}')
            self.internal_position.data = glm.vec3(value)
        else: raise TypeError(f'Node: Invalid position value type {type(value)}')
        if self.chunk: self.chunk.update()
    
    @scale.setter
    def scale(self, value: tuple | list | glm.vec3 | np.ndarray):
        if isinstance(value, glm.vec3): self.internal_scale.data = glm.vec3(value)
        elif isinstance(value, tuple) or isinstance(value, list) or isinstance(value, np.ndarray):
            if len(value) != 3: raise ValueError(f'Node: Invalid number of values for scale. Expected 3, got {len(value)}')
            self.internal_scale.data = glm.vec3(value)
        else: raise TypeError(f'Node: Invalid scale value type {type(value)}')
        if self.chunk: self.chunk.update()

    @rotation.setter
    def rotation(self, value: tuple | list | glm.vec3 | glm.quat | glm.vec4 | np.ndarray):
        if isinstance(value, glm.quat) or isinstance(value, glm.vec4) or isinstance(value, glm.vec3): self._rotation = glm.quat(value)
        elif isinstance(value, tuple) or isinstance(value, list) or isinstance(value, np.ndarray):
            if len(value) == 3: self._rotation = glm.quat(glm.vec3(*value))
            elif len(value) == 4: self._rotation = glm.quat(*value)
            else: raise ValueError(f'Node: Invalid number of values for rotation. Expected 3 or 4, got {len(value)}')
        else: raise TypeError(f'Node: Invalid rotation value type {type(value)}')
        if self.chunk: self.chunk.update()
        
    @forward.setter
    def forward(self, value: tuple | list | glm.vec3 | np.ndarray):
        if isinstance(value, glm.vec3): self._forward = glm.vec3(value)
        elif isinstance(value, tuple) or isinstance(value, list) or isinstance(value, np.ndarray):
            if len(value) != 3: raise ValueError(f'Node: Invalid number of values for forward. Expected 3, got {len(value)}')
            self._forward = glm.vec3(value)
        else: raise TypeError(f'Node: Invalid forward value type {type(value)}')
        
    # TODO add setter for Mesh
    
    # TODO add setter for Material
    
    @velocity.setter
    def velocity(self, value: tuple | list | glm.vec3 | np.ndarray):
        if isinstance(value, glm.vec3): self._velocity = glm.vec3(value)
        elif isinstance(value, tuple) or isinstance(value, list) or isinstance(value, np.ndarray):
            if len(value) != 3: raise ValueError(f'Node: Invalid number of values for velocity. Expected 3, got {len(value)}')
            self._velocity = glm.vec3(value)
        else: raise TypeError(f'Node: Invalid velocity value type {type(value)}')
        
    @rotational_velocity.setter
    def rotational_velocity(self, value: tuple | list | glm.vec3 | np.ndarray):
        if isinstance(value, glm.vec3): self._rotational_velocity = glm.vec3(value)
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
        if isinstance(value, str): self.collider.collision_group = value
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

    @x.setter
    def x(self, value: int | float):
        if isinstance(value, int) or isinstance(value, float): self.internal_position.data.x = value
        else: raise TypeError(f'Node: Invalid positional x value type {type(value)}')
        
    @y.setter
    def y(self, value: int | float):
        if isinstance(value, int) or isinstance(value, float): self.internal_position.data.y = value
        else: raise TypeError(f'Node: Invalid positional y value type {type(value)}')
        
    @z.setter
    def z(self, value: int | float):
        if isinstance(value, int) or isinstance(value, float): self.internal_position.data.z = value
        else: raise TypeError(f'Node: Invalid positional z value type {type(value)}')