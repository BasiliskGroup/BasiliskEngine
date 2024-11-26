import glm

class Node():
    position: glm.vec3
    """The position of the node in meters with swizzle xyz"""
    scale: glm.vec3
    """The scale of the node in meters in each direction"""
    rotation: glm.quat
    """The rotation of the node"""
    forward: glm.vec3
    """The forward facing vector of the node"""

    mesh: any
    """The mesh of the node stored as a basilisk mesh object"""
    material: any
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
    group: str
    """Nodes of the same collision group do not collide with each other"""

    name: str
    """The name of the node for reference"""  
    tags: list[str]
    """Tags are used to sort nodes into separate groups"""

    children: list
    """List of nodes that this node is a parent of"""

    def __init__(self) -> None:
        """
        Basilisk node object. 
        Contains mesh data, translation, material, physics, collider, and descriptive information. 
        Base building block for populating a Basilisk scene.
        """

        ...