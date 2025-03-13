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

    def __init__(self, engine) -> None:
        """
        Handles all the shader programs in a basilisk scene
        """
        
        # Back references
        self.engine = engine
        self.ctx    = engine.ctx

        # Initalize dictionaries
        self.shaders = set()

        # Load a default shader
        self.default_shader = Shader(self.engine, self.engine.root + '/shaders/batch.vert', self.engine.root + '/shaders/batch.frag')
        self.default_shader.hash = self.default_shader.hash + hash('engine_shader')
        self.add(self.default_shader)
        setattr(self.engine, "_shader", self.default_shader)

    def add(self, shader: Shader) -> None:
        """
        Creates a shader program from a file name.
        Parses through shaders to identify uniforms and save for writting
        """


        if not shader: return None
        if shader in self.shaders: return shader

        self.shaders.add(shader)
        
        if self.engine.material_handler:
            self.engine.material_handler.write()
            self.engine.material_handler.image_handler.write()

        return shader

    def get_uniforms_values(self, scene: ...) -> None:
        """
        Gets uniforms from various parts of the scene.
        These values are stored and used in write_all_uniforms and update_uniforms.
        This is called by write_all_uniforms and update_uniforms, so there is no need to call this manually.
        """
        
        self.uniform_values = {
            'projectionMatrix' : scene.camera.m_proj,
            'viewMatrix' : scene.camera.m_view,
            'cameraPosition' : scene.camera.position,
            'viewportDimensions' : glm.vec2(self.engine.win_size),
        }

    def write(self, scene: ...) -> None:
        """
        Writes all of the uniforms in every shader program.
        """

        self.get_uniforms_values(scene)
        for uniform in self.uniform_values:
            for shader in self.shaders:
                if not uniform in shader.uniforms: continue  # Does not write uniforms not in the shader
                shader.write(uniform, self.uniform_values[uniform])

    def release(self) -> None:
        """
        Releases all shader programs in handler
        """
        
        [shader.__del__() for shader in self.shaders]