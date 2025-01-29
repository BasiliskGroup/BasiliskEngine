import glm

class PhysicsBody():
    physics_engine: ...
    """Back reference to the parent physics engine"""
    mass: float
    """The mass of the physics body in kg"""

    def __init__(self, mass:float=1.0) -> None:
        self.physics_engine = None
        self.mass = mass 
        
    def get_delta_velocity(self, dt: float) -> glm.vec3:
        """
        Returns the chnage in translational velocity from constant accelerations and forces
        """
        if not self.physics_engine: return None
        
        dv = glm.vec3(0, 0, 0)
        for acceleration in self.physics_engine.accelerations: dv += acceleration * dt
        for force in self.physics_engine.forces: dv += force / self.mass * dt
        return dv
        
    def get_delta_rotational_velocity(self, dt: float) -> glm.quat:
        """
        Returns the delta rotation quaternion from constant accelerations and forces
        """
        if not self.physics_engine: return None
        
        dw = glm.vec3(0, 0, 0)
        for rotational_acceleration in self.physics_engine.rotational_accelerations: dw += rotational_acceleration * dt
        # TODO add torques
        return dw
    
    @property
    def physics_engine(self): return self._physics_engine

    @physics_engine.setter
    def physics_engine(self, value): 
        self._physics_engine = value
        if value: value.add(self)

    def __repr__(self) -> str:
        return f'<Physics Body| {self.mass}>'