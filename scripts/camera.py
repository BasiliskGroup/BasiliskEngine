import glm
import pygame as pg

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
    def __init__(self, engine, position=(0, 0, 20), yaw=-90, pitch=0) -> None:
        # Stores the engine to acces viewport and inputs
        self.engine = engine
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
    
# camera that will be attached to node
class FollowCamera(Camera):
    def __init__(self, engine, radius, yaw=-90, pitch=0):
        self.anchor = glm.vec3(0, 0, 0)
        self.radius = radius
        super().__init__(engine, (0, 0, 0), yaw, pitch)
        
    def move(self):
        pass # does nothing since movement is locked to parent
    
    def get_view_matrix(self) -> glm.mat4x4:
        self.position = self.anchor - self.forward * self.radius
        return glm.lookAt(self.position, self.anchor, self.up)
    
class StaticCamera(Camera):
    def __init__(self, engine, position=(0, 0, 20), yaw=-90, pitch=0):
        super().__init__(engine, position, yaw, pitch)
        
    def rotate(self):
        pass
    
    def move(self):
        pass