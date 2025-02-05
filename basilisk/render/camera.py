import pygame as pg
import glm
import numpy as np

# Camera view constants
FOV = 50  # Degrees
NEAR = 0.1
FAR = 350

# Camera movement constants
SPEED = 10
SENSITIVITY = 0.15

class Camera:
    engine: ...
    """Back reference to the parent engine"""
    scene: ...
    """Back reference to the parent scene"""
    aspect_ratio: float
    """Aspect ratio of the engine window"""
    position: glm.vec3
    """Location of the camera (maters)"""

    def __init__(self, position=(0, 0, 20), yaw=-90, pitch=0) -> None:
        """
        Camera object to get view and projection matricies. Movement built in
        """
        # Back references
        self.scene  = None
        self.engine = None
        # The initial aspect ratio of the screen
        self.aspect_ratio = 1.0
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
        """
        Updates the camera view matrix
        """
        
        self.update_camera_vectors()
        self.m_view = self.get_view_matrix()

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
    
    def look_at(self, other) -> None:
        forward = glm.normalize(other.position - self.position)
        self.yaw = np.degrees(np.arctan2(forward.z, forward.x))
        self.pitch = np.degrees(np.arctan2(forward.y, np.sqrt(forward.x ** 2 + forward.z ** 2)))

    def __repr__(self):
        return f'<Basilisk Camera | Position: {self.position}, Direction: {self.forward}>'

    @property
    def scene(self): return self._scene
    @property
    def position(self): return self._position
    @property
    def direction(self): return self.forward
    @property
    def horizontal(self): return glm.normalize(self.forward.xz)

    @scene.setter
    def scene(self, value):
        if value == None: return
        self._scene = value
        self.engine = self._scene.engine
        self.use()
        
    @position.setter
    def position(self, value: tuple | list | glm.vec3 | np.ndarray):
        if isinstance(value, glm.vec3): self._position = glm.vec3(value)
        elif isinstance(value, tuple) or isinstance(value, list) or isinstance(value, np.ndarray):
            if len(value) != 3: raise ValueError(f'Camera: Invalid number of values for position. Expected 3, got {len(value)}')
            self._position = glm.vec3(value)
        else: raise TypeError(f'Camera: Invalid position value type {type(value)}')
        
    @direction.setter
    def direction(self, value: tuple | list | glm.vec3 | np.ndarray):
        if isinstance(value, glm.vec3): self.direction = glm.normalize(glm.vec3(value))
        elif isinstance(value, tuple) or isinstance(value, list) or isinstance(value, np.ndarray):
            if len(value) != 3: raise ValueError(f'Camera: Invalid number of values for direction. Expected 3, got {len(value)}')
            self.forward = glm.normalize(glm.vec3(value))
        else: raise TypeError(f'Camera: Invalid direction value type {type(value)}')


class FreeCamera(Camera):
    def __init__(self, position=(0, 0, 20), yaw=-90, pitch=0):
        super().__init__(position, yaw, pitch)

    def update(self) -> None:
        """
        Updates the camera position and rotaiton based on user input
        """
        
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

    def move(self) -> None:
        """
        Checks for button presses and updates vectors accordingly. 
        """
        velocity = (SPEED + self.engine.keys[pg.K_CAPSLOCK] * 10) * self.engine.delta_time
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


class FollowCamera(FreeCamera):
    def __init__(self, parent, position=(0, 0, 20), yaw=-90, pitch=0, offset=(0, 0, 0)):
        super().__init__(position, yaw, pitch)
        self.parent = parent
        self.offest = glm.vec3(offset)
    
    def move(self) -> None:
        """
        Moves the camera to the parent node
        """

        self.position = self.parent.position + self.offest
        
class OrbitCamera(FreeCamera):
    def __init__(self, parent, position=(0, 0, 20), yaw=-90, pitch=0, distance=5, offset=(0, 0)):
        self.parent = parent
        self.distance = distance
        self.offset = glm.vec2(offset)
        super().__init__(position, yaw, pitch)

    def get_view_matrix(self) -> glm.mat4x4:
        return glm.lookAt(self.position, self.parent.position, self.up)

    def move(self) -> None:
        """
        Moves the camera to the parent node
        """

        self.position = self.parent.position - glm.normalize(self.forward) * self.distance

class StaticCamera(Camera):
    def __init__(self, position=(0, 0, 20), yaw=-90, pitch=0):
        super().__init__(position, yaw, pitch)