import glm
from .physics_body import PhysicsBody

class PhysicsEngine():
    physics_bodies: list[PhysicsBody]
    """Contains all the physics bodies controlled by this physics engine"""
    accelerations: list[glm.vec3]
    """Contains constant accelerations to be applied to physics bodies"""
    rotational_accelerations: list[glm.vec3]
    """Contains constant rotational accelerations to be applied to physics bodies"""
    forces: list[glm.vec3]
    """Contains constant forces to be applied to physics bodies"""
    torques: list[glm.vec3]
    """Contains constant rotational accelerations to be applied to physics bodies"""
    
    def __init__(self, accelerations: list[glm.vec3] = None, rotational_accelerations: list[glm.vec3] = None, forces: list[glm.vec3] = None, torques: list[glm.vec3] = None) -> None:
        self.physics_bodies = []
        self.accelerations = accelerations if accelerations else [glm.vec3(0, -9.8, 0)]
        self.rotational_accelerations = rotational_accelerations if rotational_accelerations else []
        self.forces = forces if forces else []
        self.torques = torques if torques else []
        
    def add(self, physics_body: PhysicsBody) -> PhysicsBody:
        """
        Adds a physics body to the physics engine and returns it
        """
        self.physics_bodies.append(physics_body)
        return physics_body
    
    def remove(self, physics_body: PhysicsBody) -> None:
        """
        Removes the physics body from the physics engine and disconnects it from the engine
        """
        if physics_body in self.physics_bodies: 
            self.physics_bodies.remove(physics_body)
            physics_body.physics_engine = None