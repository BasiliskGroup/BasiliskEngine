import moderngl as mgl
import glm
from .shader import Shader


class ShaderHandler:
    engine: ...
    """Back reference to the parent engine"""
    scene: ...
    """Back reference to the parent scene"""
    ctx: mgl.Context
    """Back reference to the parent context"""
    shaders: set
    """Dictionary containing all the shaders"""
    uniform_values: dict = {}
    """Dictionary containing uniform values"""    

    def __init__(self, scene) -> None:
        """
        Handles all the shader programs in a basilisk scene
        """
        
        # Back references
        self.scene  = scene
        self.engine = scene.engine
        self.ctx    = scene.engine.ctx

        # Initalize dictionaries
        self.shaders = set()
        self.add(self.engine.shader)

    def add(self, shader: Shader) -> None:
        """
        Creates a shader program from a file name.
        Parses through shaders to identify uniforms and save for writting
        """


        if not shader: return None
        if shader in self.shaders: return shader

        self.shaders.add(shader)
        
        if self.scene.material_handler:
            self.scene.light_handler.write()
            self.scene.material_handler.write()
            self.scene.material_handler.image_handler.write()

        return shader

    def get_uniforms_values(self) -> None:
        """
        Gets uniforms from various parts of the scene.
        These values are stored and used in write_all_uniforms and update_uniforms.
        This is called by write_all_uniforms and update_uniforms, so there is no need to call this manually.
        """
        
        self.uniform_values = {
            'projectionMatrix' : self.scene.camera.m_proj,
            'viewMatrix' : self.scene.camera.m_view,
            'cameraPosition' : self.scene.camera.position,
        }

    def write(self) -> None:
        """
        Writes all of the uniforms in every shader program.
        """

        self.get_uniforms_values()
        for uniform in self.uniform_values:
            for shader in self.shaders:
                if not uniform in shader.uniforms: continue  # Does not write uniforms not in the shader
                shader.write(uniform, self.uniform_values[uniform])

    def release(self) -> None:
        """
        Releases all shader programs in handler
        """
        
        [shader.__del__() for shader in self.shaders]