import numpy as np
import moderngl as mgl
from PIL import Image


class Frame:
    program: mgl.Program=None
    vbo: mgl.Buffer=None
    vao: mgl.VertexArray=None
    frame_texture: mgl.Texture=None
    depth_texture: mgl.Texture=None
    framebuffer: mgl.Framebuffer=None
    pingpong_frame_texture: mgl.Texture=None
    pingpong_depth_texture: mgl.Texture=None
    pingpong_framebuffer: mgl.Framebuffer=None
    postprocess: dict=None

    def __init__(self, scene) -> None:
        """
        Basilisk render destination. 
        Can be used to render to the screen or for headless rendering
        """

        self.scene  = scene
        self.engine = scene.engine
        self.ctx    = scene.ctx

        
        self.load_program()
        self.set_textures()
        self.set_renderer()

        self.postprocess = {}
        self.load_post_shader('frame', 'filter')

    def render(self):
        """
        Renders the current frame to the screen
        """

        # self.apply_postprocess('filter')
        
        self.ctx.screen.use()
        self.program['screenTexture'] = 0
        self.framebuffer.color_attachments[0].use(location=0)
        self.vao.render()

    def use(self):
        """
        Uses the frame as a render target
        """
        
        self.framebuffer.use()
        self.framebuffer.clear()

    def save(self, destination: str=None):
        """
        Saves the frame as an image to the given file destination
        """

        path = destination if destination else 'screenshot'

        data = self.framebuffer.read(components=3, alignment=1)
        img = Image.frombytes('RGB', self.framebuffer.size, data).transpose(Image.FLIP_TOP_BOTTOM)
        img.save(f'{path}.png')


    def load_program(self) -> None:
        """
        Loads the frame shaders
        """

        # Release any existing memory
        if self.program: self.program.release()
        
        # Read the shaders
        with open(self.engine.root + '/shaders/frame.vert') as file:
            vertex_shader = file.read()
        with open(self.engine.root + '/shaders/frame.frag') as file:
            fragment_shader = file.read()

        # Create the program
        self.program = self.ctx.program(vertex_shader=vertex_shader, fragment_shader=fragment_shader)

    def load_post_shader(self, vert: str, frag: str) -> None:
        """
        Loads a post processing shader
        """

        # Read the shaders
        with open(self.engine.root + f'/shaders/{vert}.vert') as file:
            vertex_shader = file.read()
        with open(self.engine.root + f'/shaders/{frag}.frag') as file:
            fragment_shader = file.read()

        # Create the program
        program = self.ctx.program(vertex_shader=vertex_shader, fragment_shader=fragment_shader)
        self.postprocess[frag] = self.ctx.vertex_array(program, [(self.vbo, '3f 2f', 'in_position', 'in_uv')], skip_errors=True)

    def apply_postprocess(self, shader: str):
        self.pingpong_framebuffer.use()
        self.pingpong_framebuffer.clear()
        self.postprocess[shader].program['screenTexture'] = 0
        self.framebuffer.color_attachments[0].use(location=0)
        self.postprocess[shader].render()
        

        temp = self.framebuffer
        self.framebuffer = self.pingpong_framebuffer
        self.pingpong_framebuffer = temp

        # self.use()
        # self.postprocess[shader].program['screenTexture'] = 0
        # self.pingpong_frame_texture.use(location=0)
        # self.vao.render()

        
    def set_textures(self, viewport: tuple=None) -> None:
        """
        Sets the framebuffer textures
        """

        # Release any existing memory in case of a resize
        if self.frame_texture: self.frame_texture.release()
        if self.depth_texture: self.depth_texture.release()
        if self.framebuffer:   self.framebuffer.release()
        if self.pingpong_frame_texture: self.pingpong_frame_texture.release()
        if self.pingpong_depth_texture: self.pingpong_depth_texture.release()
        if self.pingpong_framebuffer:   self.pingpong_framebuffer.release()

        # Get the size from the engine window if the not specified by the function call
        size = viewport if viewport else self.engine.win_size

        # Create textures and frame buffer object
        self.frame_texture = self.ctx.texture(size, components=4)
        self.depth_texture = self.ctx.depth_texture(size)
        self.framebuffer   = self.ctx.framebuffer([self.frame_texture], self.depth_texture)
        self.pingpong_frame_texture = self.ctx.texture(size, components=4)
        self.pingpong_depth_texture = self.ctx.depth_texture(size)
        self.pingpong_framebuffer   = self.ctx.framebuffer([self.pingpong_frame_texture], self.pingpong_depth_texture)

    def set_renderer(self) -> None:
        """
        Sets the vertex data and vao for the frame
        """
        
        # Release any existing memory
        if self.vbo: self.vbo.release()
        if self.vao: self.vao.release()

        # Vertex and index info for the frame
        verticies = [[-1, -1, 0], [ 1, -1, 0], [ 1, 1, 0], [-1, 1, 0]]
        indicies  = [(3, 0, 1), (2, 3, 1)]
        uv_verticies = [(0, 0), (1, 0), (1, 1), (0, 1)]
        uv_indicies  = [(3, 0, 1),(2, 3, 1)]
        
        # Format the data
        vertex_data = [verticies[ind] for triangle in indicies for ind in triangle]
        vertex_data = np.array(vertex_data, dtype='f4')
        uv_data     = [uv_verticies[ind] for triangle in uv_indicies for ind in triangle]
        uv_data     = np.array(uv_data, dtype='f4')

        vertex_data = np.hstack([vertex_data, uv_data])

        # Create moderngl objects
        self.vbo = self.ctx.buffer(vertex_data)
        self.vao = self.ctx.vertex_array(self.program, [(self.vbo, '3f 2f', 'in_position', 'in_uv')], skip_errors=True)
        
    def __del__(self) -> None:
        """
        Releases memory used by the frame
        """
        
        if self.program:       self.program.release()
        if self.vbo:           self.vbo.release()
        if self.vao:           self.vao.release()
        if self.frame_texture: self.frame_texture.release()
        if self.depth_texture: self.depth_texture.release()
        if self.framebuffer:   self.framebuffer.release()
        if self.pingpong_frame_texture: self.frame_texture.release()
        if self.pingpong_depth_texture: self.depth_texture.release()
        if self.pingpong_framebuffer:   self.framebuffer.release()