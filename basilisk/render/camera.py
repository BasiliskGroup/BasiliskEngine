import pygame as pg
import glm
import numpy as np
from ..generic.vec3 import Vec3
from ..generic.quat import Quat


# Camera view constants
FOV = 50  # Degrees
NEAR = 0.1
FAR = 350

class Camera:
    engine: ...
    """Back reference to the parent engine"""
    scene: ...
    """Back reference to the parent scene"""
    aspect_ratio: float
    """Aspect ratio of the engine window"""
    position: glm.vec3
    """Location of the camera (maters)"""
    speed: float
    """The speed that the camera moves in space"""
    sensitivity: float
    """The speed at which the camera turns"""

    def __init__(self, position=(0, 0, 20), rotation=(1, 0, 0, 0), speed: float=10, sensitivity: float=2) -> None:
        """
        Camera object to get view and projection matricies. Movement built in
        """
        
        # Back references
        self.scene  = None
        self.engine = None
        # transformations
        self.rotation = glm.quat(rotation)
        self.position = glm.vec3(position)
        # fov
        self.fov = 50
        # The initial aspect ratio of the screen
        self.aspect_ratio = 1.0
        # View matrix
        self.m_view = self.get_view_matrix()
        # Projection matrix
        self.m_proj = self.get_projection_matrix()
        # Movement attributes
        self.speed = speed
        self.sensitivity = sensitivity

    def update(self) -> None:
        """
        Updates the camera view matrix
        """
        
        # self.update_camera_vectors()
        self.m_view = self.get_view_matrix()

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
        return glm.perspective(glm.radians(self.fov), self.aspect_ratio, NEAR, FAR)
    
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
    def rotation(self) -> glm.quat: return self._rotation
    @property
    def direction(self): return self.rotation * (0, 0, -1)
    @property
    def forward(self): return self.rotation * (0, 0, -1)
    @property
    def pitch(self): return glm.pitch(self.rotation)
    @property
    def yaw(self): return glm.yaw(self.rotation)
    @property
    def roll(self): return glm.roll(self.rotation)
    @property
    def UP(self): 
        up = (self.rotation.x, self.rotation.y, self.rotation.z)
        up = (0, 1, 0) # TODO ensure that this works with all up vectors
        return glm.normalize(up) if glm.length2(up) > 1e-7 else glm.vec3(0, 1, 0)
    @property
    def right(self): return glm.normalize(glm.cross(self.forward, self.UP))
    @property
    def up(self): return glm.normalize(glm.cross(self.right, self.forward))
    @property
    def horizontal(self): return glm.normalize(glm.cross(self.UP, self.right))
    @property
    def fov(self): return self._fov

    @scene.setter
    def scene(self, value):
        if value == None: return
        self._scene = value
        self.engine = self._scene.engine
        self.use()
        
    @position.setter
    def position(self, value: tuple | list | glm.vec3 | np.ndarray | Vec3):
        if isinstance(value, glm.vec3): self._position = glm.vec3(value)
        elif isinstance(value, Vec3): self._position = glm.vec3(value.data)
        elif isinstance(value, tuple) or isinstance(value, list) or isinstance(value, np.ndarray):
            if len(value) != 3: raise ValueError(f'Camera: Invalid number of values for position. Expected 3, got {len(value)}')
            self._position = glm.vec3(value)
        else: raise TypeError(f'Camera: Invalid position value type {type(value)}')
        
    @rotation.setter
    def rotation(self, value):
        if isinstance(value, (glm.vec3, glm.quat)): self._rotation = glm.quat(value)
        elif isinstance(value, (Vec3, Quat)): self._rotation = glm.quat(value.data)
        elif isinstance(value, tuple) or isinstance(value, list) or isinstance(value, np.ndarray):
            if not (2 < len(value) < 5): raise ValueError(f'Camera: Invalid number of values for rotation. Expected 3 or 4, got {len(value)}')
            self._position = glm.quat(value)
        else:
            try:
                self._rotation = glm.quat(value)
            except:
                raise TypeError(f'Camera: Invalid rotation value type {type(value)}')
        
    @direction.setter
    def direction(self, value: tuple | list | glm.vec3 | np.ndarray | Vec3):
        if isinstance(value, glm.vec3): self.forward = glm.normalize(value)
        elif isinstance(value, Vec3): self.forward = glm.normalize(value.data)
        elif isinstance(value, tuple) or isinstance(value, list) or isinstance(value, np.ndarray):
            if len(value) != 3: raise ValueError(f'Camera: Invalid number of values for direction. Expected 3, got {len(value)}')
            self.forward = glm.normalize(value)
        else: raise TypeError(f'Camera: Invalid direction value type {type(value)}')
        
    @forward.setter
    def forward(self, value):
        self._rotation = glm.quatLookAt(value, self.UP)
        
    @pitch.setter
    def pitch(self, value):
        self._rotation = glm.quat((value, self.yaw, self.roll))
    
    @yaw.setter
    def yaw(self, value):
        self._rotation = glm.quat((self.pitch, value, self.roll))
    
    @roll.setter
    def roll(self, value):
        self._rotation = glm.quat((self.pitch, self.yaw, value))
    
    @UP.setter
    def UP(self, value):
        self._rotation = glm.quatLookAt(self.forward, value)
        
    @fov.setter
    def fov(self, value):
        self._fov = value
        if self.engine: self.use()


class FreeCamera(Camera):
    def __init__(self, position=(0, 0, 20), rotation=(1, 0, 0, 0)):
        super().__init__(position, rotation)

    def update(self) -> None:
        """
        Updates the camera position and rotaiton based on user input
        """
        
        self.move()
        self.rotate()
        # self.update_camera_vectors()
        self.m_view = self.get_view_matrix()

    def rotate(self) -> None:
        """
        Rotates the camera based on the amount of mouse movement.
        """
        rel_x, rel_y = self.engine.mouse.relative
        
        yaw_rotation = glm.angleAxis(self.sensitivity / 1000 * rel_x, -self.UP)
        pitch_rotation = glm.angleAxis(self.sensitivity / 1000 * rel_y, -self.right)
        new_rotation = yaw_rotation * pitch_rotation * self.rotation
        
        v_new = new_rotation * self.UP
        pitch_angle = glm.degrees(glm.acos(glm.clamp(glm.dot(v_new, self.UP), -1.0, 1.0)))
        self.rotation = new_rotation if pitch_angle < 89 else yaw_rotation * self.rotation

    def move(self) -> None:
        """
        Checks for button presses and updates vectors accordingly. 
        """
        velocity = (self.speed + self.engine.keys[pg.K_CAPSLOCK] * 10) * self.engine.delta_time
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
            
class FixedCamera(FreeCamera):
    def __init__(self, position=(0, 0, 20), rotation=(1, 0, 0, 0)):
        super().__init__(position, rotation)

    def move(self): pass

class FollowCamera(FreeCamera):
    def __init__(self, parent, position=(0, 0, 20), rotation=(1, 0, 0, 0), offset=(0, 0, 0)):
        super().__init__(position, rotation)
        self.parent = parent
        self.offest = glm.vec3(offset)
    
    def move(self) -> None:
        """
        Moves the camera to the parent node
        """

        self.position = self.parent.position + self.offest
        
class OrbitCamera(FreeCamera):
    def __init__(self, parent, position=(0, 0, 20), rotation=(1, 0, 0, 0), distance=5, offset=(0, 0)):
        self.parent = parent
        self.distance = distance
        self.offset = glm.vec2(offset)
        super().__init__(position, rotation)

    def get_view_matrix(self) -> glm.mat4x4:
        return glm.lookAt(self.position, self.parent.position, self.up)

    def move(self) -> None:
        """
        Moves the camera to the parent node
        """
        self.position = self.parent.position - glm.normalize(self.forward) * self.distance

class StaticCamera(Camera):
    def __init__(self, position=(0, 0, 20), rotation=(1, 0, 0, 0)):
        super().__init__(position, rotation)