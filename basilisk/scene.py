import moderngl as mgl


class Scene():
    engine: any
    """Parent engine of the scene"""
    ctx: mgl.Context
    """Reference to the engine context"""

    def __init__(self) -> None:
        """
    Basilisk scene object. Contains all nodes for the scene
    """
        ...

    def update(self) -> None:
        """
        Updates the physics and in the scene
        """
        
        ...

    def render(self) -> None:
        """
        Renders all the nodes with meshes in the scene
        """

        ...
    
    def set_engine(self, engine: any) -> None:
        """
        Sets the back references to the engine and creates handlers with the context
        """
        self.engine = engine
        self.ctx = engine.ctx