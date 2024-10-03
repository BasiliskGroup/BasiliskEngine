from scripts.render.vbo_handler import VBOHandler
from scripts.render.shader_handler import ShaderHandler


class VAOHandler:
    """
    Stores VBO and shader handlers. Creates VAOs
    """
    def __init__(self, project):
        self.project = project
        self.ctx = self.project.ctx
        self.frame_texture = None
        self.depth_texture = None
        self.framebuffer   = None
    
        self.shader_handler = ShaderHandler(self.project)
        self.vbo_handler = VBOHandler(self.ctx)

        self.generate_framebuffer()

        self.vaos = {}
        self.add_vao('frame', 'frame', 'frame')

    def add_vao(self, name: str='cube', program_key: str='default', vbo_key: str='cube'):
        """
        Adds a new VAO with a program and VBO. Creates an empty instance buffer
        """
        # Get program an vbo
        program = self.shader_handler.programs[program_key]
        vbo = self.vbo_handler.frame_vbo if vbo_key == 'frame' else self.vbo_handler.vbos[vbo_key]

        # Make the VAO
        vao = self.ctx.vertex_array(program, [(vbo.vbo, vbo.format, *vbo.attribs)], skip_errors=True)

        # Save th VAO
        self.vaos[name] = vao
    
    def generate_framebuffer(self):
        # Avoid a bad memory leak lmao
        if self.frame_texture : self.frame_texture.release()
        if self.depth_texture : self.depth_texture.release()
        if self.framebuffer   : self.framebuffer.release()

        self.frame_texture = self.ctx.texture(self.project.engine.win_size, components=4)
        self.depth_texture = self.ctx.depth_texture(self.project.engine.win_size)
        self.framebuffer   = self.ctx.framebuffer([self.frame_texture], self.depth_texture)

    def release(self):
        """
        Releases all VAOs and shader programs in handler
        """
        
        for vao in self.vaos.values():
            vao.release()

        self.vbo_handler.release()
        self.shader_handler.release()