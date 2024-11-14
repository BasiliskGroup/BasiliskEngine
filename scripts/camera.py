import glm
import pygame as pg

from scripts.generic.math_functions import get_model_matrix, moller_trumbore

# Camera view constants
FOV = 50  # Degrees
NEAR = 0.1
FAR = 350

# Camera movement constants
SPEED = 25
SENSITIVITY = 0.15

class Camera:
    """
    Camera object to get view and projection matricies. Movement built in
    """
    def __init__(self, engine, scene, position=(0, 0, 20), yaw=-90, pitch=0) -> None:
        # Stores the engine to acces viewport and inputs
        self.engine = engine
        self.scene  = scene
        # The initial aspect ratio of the screen
        self.aspect_ratio = self.engine.win_size[0] / self.engine.win_size[1]
        # Position
        self.position = glm.vec3(position)
        # k vector for vertical movement
        self.UP = glm.vec3(0, 1, 0)
        # Movement vectors
        self.up = glm.vec3(0, 1, 0)
        self.right = glm.vec3(1, 0, 0)
        self.forward = glm.vec3(0, 0, -1)
        # Look directions in degrees
        self.yaw = yaw
        self.pitch = pitch
        # View matrix
        self.m_view = self.get_view_matrix()
        # Projection matrix
        self.m_proj = self.get_projection_matrix()

    def update(self) -> None:
        self.move()
        self.rotate()
        self.update_camera_vectors()
        self.m_view = self.get_view_matrix()

    def rotate(self) -> None:
        """
        Rotates the camera based on the amount of mouse movement.
        """
        rel_x, rel_y = pg.mouse.get_rel()
        self.yaw += rel_x * SENSITIVITY
        self.pitch -= rel_y * SENSITIVITY
        self.yaw = self.yaw % 360
        self.pitch = max(-89, min(89, self.pitch))

    def update_camera_vectors(self) -> None:
        """
        Computes the forward vector based on the pitch and yaw. Computes horizontal and vertical vectors with cross product.
        """
        yaw, pitch = glm.radians(self.yaw), glm.radians(self.pitch)

        self.forward.x = glm.cos(yaw) * glm.cos(pitch)
        self.forward.y = glm.sin(pitch)
        self.forward.z = glm.sin(yaw) * glm.cos(pitch)

        self.forward = glm.normalize(self.forward)
        self.right = glm.normalize(glm.cross(self.forward, self.UP))
        self.up = glm.normalize(glm.cross(self.right, self.forward))

    def move(self) -> None:
        """
        Checks for button presses and updates vectors accordingly. 
        """
        velocity = SPEED * self.engine.dt
        keys = self.engine.keys
        if keys[pg.K_w]:
            self.position += glm.normalize(glm.vec3(self.forward.x, 0, self.forward.z)) * velocity
        if keys[pg.K_s]:
            self.position -= glm.normalize(glm.vec3(self.forward.x, 0, self.forward.z)) * velocity
        if keys[pg.K_a]:
            self.position -= self.right * velocity
        if keys[pg.K_d]:
            self.position += self.right * velocity
        if keys[pg.K_SPACE]:
            self.position += self.UP * velocity
        if keys[pg.K_LSHIFT]:
            self.position -= self.UP * velocity

    def use(self):
        # Updated aspect ratio of the screen
        self.aspect_ratio = self.engine.win_size[0] / self.engine.win_size[1]
        # View matrix
        self.m_view = self.get_view_matrix()
        # Projection matrix
        self.m_proj = self.get_projection_matrix()

    def get_view_matrix(self) -> glm.mat4x4:
        return glm.lookAt(self.position, self.position + self.forward, self.up)

    def get_projection_matrix(self) -> glm.mat4x4:
        return glm.perspective(glm.radians(FOV), self.aspect_ratio, NEAR, FAR)
    
    def get_params(self) -> tuple:
        return self.engine, self.position, self.yaw, self.pitch
    
    def get_model_node_at(self, position:glm.vec3=None, forward:glm.vec3=None, max_distance:float=1e5, has_collider:bool=False, has_physics_body:bool=False, material:str=None) -> tuple:
        if not forward:  forward  = self.forward
        if not position: position = self.position
        forward = glm.normalize(forward)
        
        # return best_node
        nodes = []
        for root in self.scene.node_handler.nodes: nodes.extend(root.get_nodes(True, has_collider, has_physics_body, material))
        best_distance, best_point, best_node = max_distance, None, None
        
        for node in nodes:
            # if glm.dot(forward, node.position - self.position) < 0 and node.scale.x * node.scale.y * node.scale.z < glm.length(node.position - self.position) ** 2: continue
            
            # get model matrix & convert points
            model          = node.model
            model_matrix   = get_model_matrix(model.position, model.scale, model.rotation)
            world_vertices = [glm.vec3(model_matrix * glm.vec4(*vert, 1)) for vert in self.scene.model_handler.vbos[model.vbo].unique_points]
            
            # get nearest point
            for triangle in self.scene.model_handler.vbos[model.vbo].indicies:
                intersection = moller_trumbore(position, forward, [world_vertices[t] for t in triangle])
                if not intersection: continue
                distance = glm.length(intersection - position)
                if distance < best_distance:
                    best_distance = distance
                    best_point    = intersection
                    best_node     = node
                    
        return best_node, best_point
    
# camera that will be attached to node
class FollowCamera(Camera):
    def __init__(self, engine, radius, scene, yaw=-90, pitch=0):
        self.anchor = glm.vec3(0, 0, 0)
        self.radius = radius
        self.scene = scene
        super().__init__(engine, scene, (0, 0, 0), yaw, pitch)
        
    def move(self):
        pass # does nothing since movement is locked to parent
    
    def get_view_matrix(self) -> glm.mat4x4:
        distance = self.radius
        #node, point = self.get_model_node_at(position=self.position, forward=self.forward, has_collider=True, max_distance=self.radius * 10) # self.position - self.anchor
        # print(node, point)
        # if point: distance = glm.length(self.anchor - point)
        self.position = self.anchor - self.forward * distance
        return glm.lookAt(self.position, self.anchor, self.up)
    
class StaticCamera(Camera):
    def __init__(self, engine, position=(0, 0, 20), yaw=-90, pitch=0):
        super().__init__(engine, position, yaw, pitch)
        
    def rotate(self):
        pass
    
    def move(self):
        pass