import moderngl as mgl
from .image import Image

attribute_mappings = {
    'in_position'  : [0, 1, 2],
    'in_uv'        : [3, 4],
    'in_normal'    : [5, 6, 7],
    'in_tangent'   : [8, 9, 10],
    'in_bitangent' : [11, 12, 13],
    'obj_position' : [14, 15, 16],
    'obj_rotation' : [17, 18, 19, 20],
    'obj_scale'    : [21, 22, 23],
    'obj_material' : [24],
}


class Shader:
    program: mgl.Program=None
    """Shader program for the vertex and fragment shader"""
    vertex_shader: str
    """String representation of the vertex shader"""
    fragment_shader: str
    """String representation of the vertex shader"""
    uniforms: list[str]=[]
    """List containg the names of all uniforms in the shader"""
    attribute_indices: list[int]
    """List of indices that map all possible shader attributes to the ones used byu the shader"""
    fmt: str
    """String representation of the format for building vaos"""
    attributes: list[str]
    """List representation of the attributes for building vaos"""

    def __init__(self, engine, vert: str=None, frag: str=None) -> None:
        """
        Basilisk shader object. Contains shader program and shader attrbibute/uniform information
        Args:
            vert: str=None
                Path to the vertex shader. Defaults to internal if none is given
            frag: str=None
                Path to the fragment shader. Defaults to internal if none is given    
        """

        self.engine = engine
        self.ctx    = engine.ctx

        # Default class attributes values
        self.uniforms          = []
        self.attribute_indices = []
        self.fmt               = ''
        self.attributes        = []
        self.bindings = 1

        # Default vertex and fragment shaders
        if vert == None: vert = self.engine.root + '/shaders/batch.vert'
        if frag == None: frag = self.engine.root + '/shaders/batch.frag'

        # Read the shaders
        with open(vert) as file:
            self.vertex_shader = file.read()
        with open(frag) as file:
            self.fragment_shader = file.read()
        
        # Hash value for references
        if vert == None and frag == None:
            self.hash = hash((self.vertex_shader, self.fragment_shader, 'default'))
        else:
            self.hash = hash((self.vertex_shader, self.fragment_shader))

        # Create a string of all lines in both shaders
        lines = f'{self.vertex_shader}\n{self.fragment_shader}'.split('\n')

        # Parse through shader to find uniforms and attributes
        for line in lines:
            tokens = line.strip().split(' ')

            # Add uniforms
            if tokens[0] == 'uniform' and len(tokens) > 2:
                self.uniforms.append(tokens[-1][:-1])

            # Add attributes
            if tokens[0] == 'layout' and len(tokens) > 2 and 'in' in line:
                self.attributes.append(tokens[-1][:-1])

                # Get the number of flots the attribute takes
                if any(list(map(lambda x: x in tokens, ['float', 'int']))): n = 1
                elif any(list(map(lambda x: x in tokens, ['vec2']))): n = 2
                elif any(list(map(lambda x: x in tokens, ['vec3']))): n = 3
                elif any(list(map(lambda x: x in tokens, ['vec4']))): n = 4
                else: n = 1
                self.fmt += f'{n}f '

                if tokens[-1][:-1] in attribute_mappings:
                    indices = attribute_mappings[tokens[-1][:-1]]
                else:
                    indices = [0 for i in range(n)]
                self.attribute_indices.extend(indices)

        # Create a program with shaders
        self.program = self.ctx.program(vertex_shader=self.vertex_shader, fragment_shader=self.fragment_shader)

    def set_main(self, scene):
        """
        Selects a shader for use
        """
        
        self.engine.shader_handler.add(self)
        if scene.node_handler: scene.node_handler.chunk_handler.swap_default(self)

    def write(self, name: str, value) -> None:
        """
        Writes a uniform to the shader program
        """
        
        self.program[name].write(value)
    
    def bind(self, sampler: mgl.Texture | mgl.TextureArray | mgl.TextureCube | Image, name: str, slot: int=None) -> None:
        """
        Binds the given sampler to the next availible slot
        """

        # Use the next slot if no slot is given
        if not slot: slot = self.bindings; self.bindings+=1

        if isinstance(sampler, Image,): sampler = sampler.build_texture(self.ctx)

        # Use the sampler
        self.program[name] = slot
        sampler.use(location=slot)
        

    def __del__(self) -> int:
        if self.program: self.program.release()

    def __hash__(self) -> int:
        return self.hash