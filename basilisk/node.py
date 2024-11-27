import glm
from .render.mesh import Mesh

class Node():
    position: glm.vec3
    """The position of the node in meters with swizzle xyz"""
    scale: glm.vec3
    """The scale of the node in meters in each direction"""
    rotation: glm.quat
    """The rotation of the node"""
    forward: glm.vec3
    """The forward facing vector of the node"""
    mesh: Mesh
    """The mesh of the node stored as a basilisk mesh object"""
    material: None
    """The mesh of the node stored as a basilisk material object"""
    velocity: glm.vec3
    """The translational velocity of the node"""
    rotational_velocity: glm.quat 
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
    children: list
    """List of nodes that this node is a parent of"""

    def __init__(self, 
            position: glm.vec3=None, 
            scale: glm.vec3=None, 
            rotation: glm.quat=None, 
            forward: glm.vec3=None, 
            mesh: Mesh=None, 
            material: None=None, 
            velocity: glm.vec3=None, 
            rotational_velocity: glm.quat=None, 
            physics: bool=False, 
            mass: float=None, 
            collisions: bool=False, 
            collider: str='box', 
            static_friction: float=None, 
            kinetic_friction: float=None, 
            elasticity: float=None, 
            collision_group :float=None, 
            name: str='', 
            tags: list[str]=None
        ) -> None:
        """
        Basilisk node object. 
        Contains mesh data, translation, material, physics, collider, and descriptive information. 
        Base building block for populating a Basilisk scene.
        """
        
        self.position = position if position else glm.vec3(0, 0, 0)
        self.scale    = scale    if scale    else glm.vec3(1, 1, 1)
        self.rotation = rotation if rotation else glm.quat(1, 0, 0, 0)
        self.forward  = forward  if forward  else glm.vec3(1, 0, 0)
        self.mesh     = mesh     if mesh     else None # TODO add default cube mesh
        self.material = material if material else None # TODO add default base material
        self.velocity = velocity if velocity else glm.vec3(0, 0, 0)
        self.rotational_velocity = rotational_velocity if rotational_velocity else glm.quat(1, 0, 0, 0)
        
        if physics: ...
        elif mass: raise ValueError('Node cannot have mass if it does not have physics')
        
        if collisions: ...
        elif collider:         raise ValueError('Node cannot have collider mesh if it does not allow collisions')
        elif static_friction:  raise ValueError('Node cannot have static friction if it does not allow collisions')
        elif kinetic_friction: raise ValueError('Node cannot have kinetic friction if it does not allow collisions')
        elif elasticity:       raise ValueError('Node cannot have elasticity if it does not allow collisions')
        elif collision_group:  raise ValueError('Node cannot have collider group if it does not allow collisions')
        
        self.name = name
        self.tags = tags if tags else []
        self.children = []

    def __repr__(self) -> str:
        """
        Returns a string representation of the node
        """

        return f'<Bailisk Node | {self.name}, {self.mesh}, ({self.position})>'