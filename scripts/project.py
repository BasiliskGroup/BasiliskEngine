from scripts.scene import Scene
from scripts.render.vao_handler import VAOHandler
from scripts.render.texture_handler import TextureHandler
from scripts.physics.physics_handler import PhysicsHandler


class Project:
    """
    Stores, loads, and saves scene data
    """
    def __init__(self, engine, editor=False) -> None:
        # Stores the engine
        self.engine = engine
        self.ctx = engine.ctx
        # Creates physics engine
        self.physics_handler = PhysicsHandler(None)
        # Creates vao handler to be used by scenes
        self.vao_handler = VAOHandler(self)
        # Creates a texture handler
        self.texture_handler = TextureHandler(self.engine, self.vao_handler)
        # Creates scenes
        self.scenes = {0 : Scene(self.engine, self, editor=editor)}
        self.current_scene = self.scenes[0]
        self.physics_handler.scene = self.current_scene
        # Use scene
        self.current_scene.use()

    def update(self, delta_time, camera=True) -> None:
        """
        Updates the current scene        
        """
        
        if delta_time: self.physics_handler.update(delta_time)
        self.current_scene.update(camera)

    def render(self, display=True) -> None:
        """
        Renders the current scene        
        """

        self.current_scene.render(display)

    def set_scene(self, scene: str) -> None:
        """
        Sets the scene being updated and rendered
        """

        self.scenes[scene].use()
        self.current_scene = self.scenes[scene]
        self.physics_handler.scene = self.current_scene

    def release(self) -> None:
        """
        Releases all scenes in project
        """
        [scene.release() for scene in self.scenes.values()]