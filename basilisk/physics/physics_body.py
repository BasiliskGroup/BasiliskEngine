class PhysicsBody():
    mass: float
    """The mass of the physics body in kg"""

    def __init__(self, mass:float=1.0):
        self.mass = mass