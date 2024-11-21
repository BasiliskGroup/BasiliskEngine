import glm

class PhysicsBodyHandler():
    def __init__(self, scene, physics_bodies:list = None) -> None:
        self.scene           = scene
        self.physics_handler = self.scene.project.physics_handler
        self.physics_bodies  = physics_bodies if physics_bodies else []
        
    def add(self, mass:float = 1, rotation:glm.quat = glm.quat(1, 0, 0, 0), velocity:glm.vec3 = None, rotational_velocity:int = 0, axis_of_rotation:glm.vec3 = None):
        """
        Adds a physics body to the physics handler and returns it. 
        """
        # add physics body and return
        self.physics_bodies.append(PhysicsBody(self, mass, rotation, velocity, rotational_velocity, axis_of_rotation))
        return self.physics_bodies[-1]
    
    def remove(self, physics_body):
        if physics_body in self.physics_bodies: self.physics_bodies.remove(physics_body)
        del physics_body
    
class PointPhysicsBody():
    def __init__(self, physics_body_handler:PhysicsBodyHandler, mass:float = 1, velocity:glm.vec3 = None, decay:float=0.1) -> None:
        self.mass     = mass
        self.decay    = decay
        self.velocity = velocity if velocity else glm.vec3(0, 0, 0)
        self.physics_body_handler = physics_body_handler
        
    def get_delta_position(self, delta_time:float) -> glm.vec3:
        """
        gets the new position based on rk4. sets own velocity. 
        """
        # gets delta position and velocity from rk4
        delta_position, delta_velocity = self.physics_body_handler.physics_handler.get_constant_rk4(delta_time, self.velocity)
        
        # sets velocity and returns position to collection
        self.velocity += delta_velocity
        self.velocity -= self.velocity * self.decay * delta_time
        return delta_position

class PhysicsBody(PointPhysicsBody):
    def __init__(self, physics_body_handler:PhysicsBodyHandler, mass:float=1, rotation:glm.quat=glm.quat(1, 0, 0, 0), velocity:glm.vec3=None, rotational_velocity:int=0, axis_of_rotation:glm.vec3=None, decay:float=0):
        super().__init__(physics_body_handler, mass, velocity, decay)
        self.rotational_velocity = rotational_velocity
        self.axis_of_rotation    = glm.vec3(axis_of_rotation) if axis_of_rotation else glm.vec3(1, 0, 0)
        self.rotation            = rotation # will be reset in node's after init

    def get_new_rotation(self, delta_time: float):
        """
        returns the new rotation of the object after time
        """
        # quit early if rotation is super small
        if abs(self.rotational_velocity) < 1e-5: return glm.eulerAngles(self.rotation)
        
        # get change in angle
        theta = -self.rotational_velocity * delta_time * 0.5 # negative for ccw which is positive
        
        # calculate rotation quaternion
        axis_world = glm.normalize(self.axis_of_rotation) if glm.length(self.axis_of_rotation) > 0 else glm.vec3(1, 0, 0)
        rq         = glm.angleAxis(theta, axis_world)
        
        # calculate and return using quaternions
        self.rotation = self.rotation * rq
        self.rotational_velocity -= self.decay * delta_time
        return glm.eulerAngles(self.rotation)
    
    def set_rotation(self, quat): # TODO replace usage with properties
        self.rotation = quat