import glm
from ..generic.abstract_bvh import AbstractAABB as AABB
from ..generic.meshes import transform_points, get_aabb_surface_area
from ..mesh.mesh import Mesh
from .narrow.dataclasses import Collision

class Collider():
    node: ...
    """Back reference to the node"""
    collider_handler: ...
    """Back reference to the collider handler"""
    half_dimensions: glm.vec3
    """The axis aligned dimensions of the transformed mesh"""
    static_friction: float = 0.8 # input from node constructor
    """Determines the friction of the node when still: recommended 0 - 1"""  
    kinetic_friction: float = 0.3 # input from node constructor
    """Determines the friction of the node when moving: recommended 0 - 1"""
    elasticity: float = 0.1 # input from node constructor
    """Determines how bouncy an object is: recommended 0 - 1"""  
    collision_group: str # input from node constructor
    """Nodes of the same collision group do not collide with each other"""
    has_collided: bool
    """Stores whether or not the collider has been collided with in the last frame"""  
    collision_velocity: float
    """Stores the highest velocity from a collision on this collider from the last frame"""  
    collisions: dict # {node : (normal, velocity, depth)} TODO determine which variables need to be stored
    """Stores data from collisions in the previous frame"""
    top_right: glm.vec3
    """AABB most positive corner"""
    bottom_left: glm.vec3
    """AABB most negative corner"""
    aabb_surface_area: float
    """The surface area of the collider's AABB"""
    parent: AABB
    """Reference to the parent AABB in the broad BVH"""
    mesh: Mesh
    """Reference to the colliding mesh"""

    def __init__(self, node, collider_mesh: str|Mesh=None, static_friction: glm.vec3=0.7, kinetic_friction: glm.vec3=0.3, elasticity: glm.vec3=0.2, collision_group: str=None):
        self.collider_handler = None
        self.node = node
        self.static_friction = static_friction if elasticity else 0.8 # added checks to prevent floats being set to None. Also done for kinetic and elasticity
        self.mesh = collider_mesh
        self.kinetic_friction = kinetic_friction if elasticity else 0.4
        self.elasticity = elasticity if elasticity else 0.1
        self.collision_group = collision_group
        self.collision_velocity = 0
        self.collisions: list[Collision] = []
        self.parent = None
        
        # lazy update variables TODO change to distinguish between static and nonstatic objects
        self.needs_obb = True # pos, scale, rot
        self.needs_half_dimensions = True # scale, rot
        self.needs_bvh = True # pos, scale, rot
        
    def get_vertex(self, index: int) -> glm.vec3:
        """
        Gets the world space position of a vertex indicated by the index in the mesh
        """
        return glm.vec3(self.node.model_matrix * glm.vec4(*self.mesh.points[index], 1))
    
    @property
    def collider_handler(self): return self._collider_handler
    @property
    def has_collided(self): return bool(self.collisions)
    @property
    def half_dimensions(self): # TODO look for optimization
        if self.needs_half_dimensions: 
            top_right = glm.max(self.obb_points)
            self._half_dimensions = top_right - self.node.geometric_center
            self.needs_half_dimensions = False
        return self._half_dimensions
    @property
    def bottom_left(self): return self.node.geometric_center - self.half_dimensions
    @property
    def top_right(self): return self.node.geometric_center + self.half_dimensions
    @property
    def aabb_surface_area(self): return get_aabb_surface_area(self.top_right, self.bottom_left)
    @property
    def obb_points(self): 
        if self.needs_obb: 
            self._obb_points = transform_points(self.mesh.aabb_points, self.node.model_matrix)
            self.needs_obb = False
        return self._obb_points
    
    @collider_handler.setter
    def collider_handler(self, value):
        self._collider_handler = value
        if not value: return
        if self.mesh is None: self.mesh = self.node.mesh
        elif isinstance(self.mesh, Mesh): ...
        elif isinstance(self.mesh, str):
            if self.mesh =='box': self.mesh = value.cube
            else: raise ValueError(f'Incorrect built-in mesh type {self.mesh}')
        else: raise ValueError(f'Unkown type for mesh, got {type(self.mesh)}')
        value.add(self)