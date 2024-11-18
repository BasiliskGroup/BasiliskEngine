import glm
from scripts.generic.math_functions import get_aabb_collision, get_aabb_line_collision
from scripts.generic.math_functions import get_model_matrix

class Collider():
    def __init__(self, collider_handler, position:glm.vec3=None, scale:glm.vec3=None, rotation:glm.vec3=None, vbo:str='cube', static = True, elasticity:float=0.1, kinetic_friction:float=0.4, static_friction:float=0.7, group:str=None) -> None:
        # parents
        self.node             = None
        self.collider_handler = collider_handler
        # general data
        self.position = glm.vec3(position) if position else glm.vec3(0, 0, 0)
        self.scale    = glm.vec3(scale)    if scale    else glm.vec3(1, 1, 1)
        self.rotation = glm.vec3(rotation) if rotation else glm.vec3(0, 0, 0)
        # geometry
        self.vbo = vbo
        self.unique_points = self.collider_handler.vbos[vbo].unique_points
        self.update_vertices() # vertices must be created before everything else
        self.base_dimensions = glm.vec3(2, 2, 2)
        self.base_half_dimensions = self.base_dimensions / 2
        self.update_dimensions()
        self.base_geometric_center = glm.vec3(0, 0, 0)
        self.update_geometric_center()
        # collisions
        self.update_aabb()
        self.parent = None
        self.need_vertices = True
        self.group = group
        # physics
        self.base_volume      = 8
        self.static           = static
        self.elasticity       = elasticity
        self.static_friction  = 0.99 if self.static else 0.2
        self.kinetic_friction = static_friction
        
        self.has_collided = False
        self.collision_normals = {}
        
    def update_dimensions(self):
        model_matrix    = glm.mat4x4([[abs(item) for item in row] for row in get_model_matrix(glm.vec3(0, 0, 0), self.scale, self.rotation)])
        self.dimensions = glm.vec3((new := glm.mul(model_matrix, (*[float(f) for f in self.base_half_dimensions], 1)))[0], new[1], new[2]) * 2
        
    def update_vertices(self):
        model_matrix  = get_model_matrix(self.position, self.scale, self.rotation)
        self.vertices = [glm.vec3((new := glm.mul(model_matrix, (*[float(f) for f in vertex], 1)))[0], new[1], new[2]) for vertex in self.unique_points]
        
    def update_geometric_center(self):
        self.geometric_center = self.base_geometric_center + self.position
    
    def get_volume(self) -> float:
        return self.base_volume * self.scale.x * self.scale.y * self.scale.z
    
    def update_aabb(self) -> None:
        self.top_right    = self.geometric_center + self.dimensions / 2
        self.bottom_left  = self.geometric_center - self.dimensions / 2
        diff              = self.top_right - self.bottom_left
        self.surface_area = 2 * (diff.x * diff.y + diff.y * diff.z + diff.z * diff.x)
        
    # for broad stroke collision detection
    def find_sibling(self, collider, c_best, inherited):
        # compute lowest cost
        return self.get_test_surface(collider) + inherited, self
    
    def get_collided(self, collider) -> list:
        if self.node is not collider.node and get_aabb_collision(self.top_right, self.bottom_left, collider.top_right, collider.bottom_left): return [self] # do not need to check is self since they will have the same node
        return []
    
    def get_line_collided(self, point:glm.vec3, vec:glm.vec3) -> list:
        if get_aabb_line_collision(self.top_right, self.bottom_left, point, vec): return [self]
        return []
    
    def get_test_surface(self, test_volume) -> float:
        """
        Gets the surface area of the aabb and the test volume
        Args:
            test_volume (float): either an aabb or collider
        """
        top_right         = glm.max(self.top_right, test_volume.top_right)
        bottom_left       = glm.min(self.bottom_left, test_volume.bottom_left)
        diff              = top_right - bottom_left
        return 2 * (diff.x * diff.y + diff.y * diff.z + diff.z * diff.x)
    
    # position
    @property
    def position(self): 
        return self._position
    
    @position.setter
    def position(self, value):
        self.need_vertices = True
        self._position     = value
    
    # scale
    @property
    def scale(self): 
        return self._scale
    
    @scale.setter
    def scale(self, value):
        self.need_vertices = True
        self._scale        = value
        
    # rotation
    @property
    def rotation(self): return self._rotation
    
    @rotation.setter
    def rotation(self, value):
        self.need_vertices = True
        self._rotation     = value