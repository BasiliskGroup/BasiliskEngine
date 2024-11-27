import glm

class Collider():
    node: None
    """Back reference to the node"""
    collider_handler: None
    """Back reference to the collider handler"""
    half_dimensions: glm.vec3
    """The axis aligned dimensions of the transformed mesh"""
    static_friction: float = 0.8 # input from node constructor
    """Determines the friction of the node when still: recommended 0 - 1"""  
    kinetic_friction: float = 0.3 # input from node constructor
    """Determines the friction of the node when moving: recommended 0 - 1"""
    elasticity: float = 0.1 # input from node constructor
    """Determines how bouncy an object is: recommended 0 - 1"""  
    collision_group: str = None # input from node constructor
    """Nodes of the same collision group do not collide with each other"""
    has_collided: bool = False
    """Stores whether or not the collider has been collided with in the last frame"""  
    collision_velocity: float
    """Stores the highest velocity from a collision on this collider from the last frame"""  
    collisions: dict # {node : normal} TODO determine which variables need to be stored
    """Stores data from collisions in the previous frame"""
    top_right: glm.vec3
    """AABB most positive corner"""
    bottom_left: glm.vec3
    """AABB most negative corner"""
    surface_area: float
    """The surface area of the collider's AABB"""

    def __init__(self, collider_handler, node, static_friction: glm.vec3=0.7, kinetic_friction: glm.vec3=0.3, elasticity: glm.vec3=0.1, collision_group: str=None):
        self.collider_handler = collider_handler
        self.node = node
        self.static_friction = static_friction
        self.kinetic_friction = kinetic_friction
        self.elasticity = elasticity
        self.collision_group = collision_group
        self.half_dimensions = ...
        self.has_collided = ...
        self.collision_velocity = ...
        self.collisions = ...
        self.top_right = ...
        self.bottom_left = ...
        self.surface_area = ...