import numpy as np
import moderngl as mgl
from .shader import Shader
from .image import Image
from .framebuffer import Framebuffer
from .post_process import PostProcess
from .bloom import Bloom


class Frame:
    shader: Shader=None
    vbo: mgl.Buffer=None
    vao: mgl.VertexArray=None

    def __init__(self, engine, scale: float=1.0, linear_filter: bool=False) -> None:
        """
        Basilisk render destination. 
        Can be used to render to the screen or for headless rendering
        """

        self.engine = engine
        self.ctx    = engine.ctx

        # Load framebuffer
        self.output_buffer = Framebuffer(self.engine, scale=scale, n_color_attachments=2, linear_filter=linear_filter)
        self.input_buffer = Framebuffer(self.engine, scale=scale, n_color_attachments=3, linear_filter=linear_filter)
        self.ping_pong_buffer = Framebuffer(self.engine, scale=scale, n_color_attachments=3, linear_filter=linear_filter)

        # Load Shaders
        self.shader = Shader(self.engine, self.engine.root + '/shaders/frame.vert', self.engine.root + '/shaders/frame_hdr.frag')

        # Load VAO
        self.vbo = self.ctx.buffer(np.array([[-1, -1, 0, 0, 0], [1, -1, 0, 1, 0], [1, 1, 0, 1, 1], [-1, 1, 0, 0, 1], [-1, -1, 0, 0, 0], [1, 1, 0, 1, 1]], dtype='f4'))
        self.vao = self.ctx.vertex_array(self.shader.program, [(self.vbo, '3f 2f', 'in_position', 'in_uv')], skip_errors=True)
        
        self.bloom = Bloom(self) 

        # TEMP TESTING
        self.post_processes = []


    def scene_render(self, target=None) -> None:
        """
        Renders the current frame to the screen or the given target
        """

        if self.engine.event_resize: self.bloom.generate_bloom_buffers()

        
        # for process in self.post_processes:
        #     self.ping_pong_buffer = process.apply([('screenTexture', self.input_buffer)], self.ping_pong_buffer)
            
        #     temp = self.input_buffer
        #     self.input_buffer = self.ping_pong_buffer
        #     self.ping_pong_buffer = temp
        

        if self.engine.config.bloom_enabled: 
            self.bloom.render()
            self.shader.bind(self.bloom.texture, 'bloomTexture', 1)
        
        target.use() if target else self.output_buffer.use()
        self.shader.bind(self.input_buffer.texture, 'screenTexture', 0)
        self.vao.render()

    def render(self, target=None) -> None:
        """
        Renders the current frame to the screen or the given target
        """

        self.output_buffer.render(target=target)

    def use(self) -> None:
        """
        Uses the frame as a render target
        """
        
        self.input_buffer.use()

    def add_post_process(self, post_process: PostProcess) -> PostProcess:
        """
        Add a post process to the frames post process stack
        """

        self.post_processes.append(post_process)
        return post_process

    def save(self, destination: str=None) -> None:
        """
        Saves the frame as an image to the given file destination
        """

        self.output_buffer.save(destination)
    
    def clear(self):
        """
        Clears the framebuffer of the frame
        """
        
        self.input_buffer.clear()
        self.output_buffer.clear()
        self.bloom.clear()

    def bind(self, sampler: mgl.Texture | mgl.TextureArray | mgl.TextureCube | Image, name: str, slot: int=None):
        """
        Binds a texture to the fbo's shader
        """
        
        self.shader.bind(sampler, name, slot)

    def resize(self) -> None:
        """
        Resize the frame to the given size. None for window size
        """

        self.input_buffer.resize()
        self.ping_pong_buffer.resize()
        self.generate_bloom_buffers()

    def __del__(self) -> None:
        """
        Releases memory used by the frame
        """
        
        if self.vbo: self.vbo.release()
        if self.vao: self.vao.release()

    @property
    def texture(self): return self.output_buffer.texture
    @property
    def depth(self): return self.output_buffer.depth