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
    group: str = None # input from node constructor
    """Nodes of the same collision group do not collide with each other"""
    has_collided: bool = False
    """Stores whether or not the collider has been collided with in the last frame"""  
    collision_velocity: float
    """Stores the highest velocity from a collision on this collider from the last frame"""  
    collisions: dict # {node : normal} TODO determine which variables need to be stored
    top_right: glm.vec3
    bottom_left: glm.vec3
    surface_area: float
    
