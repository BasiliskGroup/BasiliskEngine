import moderngl as mgl
import glm

# Predefined uniforms that do not change each frame
single_frame_uniforms = ['m_proj']


class ShaderHandler:
    engine: ...
    """Back reference to the parent engine"""
    scene: ...
    """Back reference to the parent scene"""
    ctx: mgl.Context
    """Back reference to the parent context"""
    programs: dict = {}
    """Dictionary containing all the shaders"""
    shader_uniforms: dict = {}
    """Dictionary all the uniforms present in a shader"""
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
        self.programs = {}
        self.shader_uniforms = {}

        self.load('batch', 'basilisk/shaders/batch.vert', 'basilisk/shaders/batch.frag')
        self.load('draw', 'basilisk/shaders/draw.vert', 'basilisk/shaders/draw.frag')

    def load(self, name: str, vert_path: str, frag_path: str) -> None:
        """
        Creates a shader program from a file name.
        Parses through shaders to identify uniforms and save for writting
        """

        # Read the shaders
        with open(vert_path) as file:
            vertex_shader = file.read()
        with open(frag_path) as file:
            fragment_shader = file.read()
            
        # Create blank list for uniforms
        self.shader_uniforms[name] = []
        # Create a list of all lines in both shaders
        lines = f'{vertex_shader}\n{fragment_shader}'.split('\n')
        # Parse through shader to find uniform variables
        for line in lines:
            tokens = line.strip().split(' ')
            if tokens[0] == 'uniform' and len(tokens) > 2:
                self.shader_uniforms[name].append(tokens[2][:-1])

        # Create a program with shaders
        program = self.ctx.program(vertex_shader=vertex_shader, fragment_shader=fragment_shader)
        self.programs[name] = program

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
            for program in self.programs:
                if not uniform in self.shader_uniforms[program]: continue  # Does not write uniforms not in the shader
                self.programs[program][uniform].write(self.uniform_values[uniform])

    def release(self) -> None:
        """
        Releases all shader programs in handler
        """
        
        [program.release() for program in self.programs.values()]