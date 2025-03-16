import numpy as np
import moderngl as mgl
import glm
from .shader import Shader
from .framebuffer import Framebuffer
from .post_process import PostProcess

class Frame:
    shader: Shader=None
    vbo: mgl.Buffer=None
    vao: mgl.VertexArray=None
    framebuffer: mgl.Framebuffer=None

    def __init__(self, engine, scale: float=1.0, linear_filter: bool=False) -> None:
        """
        Basilisk render destination. 
        Can be used to render to the screen or for headless rendering
        """

        self.engine = engine
        self.ctx    = engine.ctx


        # Load framebuffer
        self.framebuffer = Framebuffer(self.engine, scale=scale, n_color_attachments=2, linear_filter=linear_filter)
        self.ping_pong_buffer = Framebuffer(self.engine, scale=scale, n_color_attachments=2, linear_filter=linear_filter)

        self.framebuffer.texture.repeat_x = False
        self.framebuffer.texture.repeat_y = False


        self.bloom_depth = 6
        self.generate_bloom_buffers(self.bloom_depth)

        # Load Shaders
        self.shader = Shader(self.engine, self.engine.root + '/shaders/frame.vert', self.engine.root + '/shaders/bloom_frame.frag')
        self.engine.shader_handler.add(self.shader)

        # Load VAO
        self.vbo = self.ctx.buffer(np.array([[-1, -1, 0, 0, 0], [1, -1, 0, 1, 0], [1, 1, 0, 1, 1], [-1, 1, 0, 0, 1], [-1, -1, 0, 0, 0], [1, 1, 0, 1, 1]], dtype='f4'))
        self.vao = self.ctx.vertex_array(self.shader.program, [(self.vbo, '3f 2f', 'in_position', 'in_uv')], skip_errors=True)
        
        # Load bloom tools
        self.downsample_shader = Shader(self.engine, self.engine.root + '/shaders/frame.vert', self.engine.root + '/shaders/bloom_downsample.frag')
        self.upsample_shader   = Shader(self.engine, self.engine.root + '/shaders/frame.vert', self.engine.root + '/shaders/bloom_upsample.frag')
        
        self.engine.shader_handler.add(self.downsample_shader)
        self.engine.shader_handler.add(self.upsample_shader)

        self.downsample_vao = self.ctx.vertex_array(self.downsample_shader.program, [(self.vbo, '3f 2f', 'in_position', 'in_uv')], skip_errors=True)
        self.upsample_vao   = self.ctx.vertex_array(self.upsample_shader.program, [(self.vbo, '3f 2f', 'in_position', 'in_uv')], skip_errors=True)

        # TEMP TESTING
        self.post_processes = []


    def render(self) -> None:
        """
        Renders the current frame to the screen
        """

        if self.engine.event_resize: self.generate_bloom_buffers()

        for process in self.post_processes:
            self.ping_pong_buffer = process.apply(self.framebuffer, self.ping_pong_buffer)
            
            temp = self.framebuffer
            self.framebuffer = self.ping_pong_buffer
            self.ping_pong_buffer = temp
        
        self.render_bloom()

        self.ctx.screen.use()
        self.shader.bind(self.framebuffer.texture, 'screenTexture', 0)
        self.shader.bind(getattr(self, f'upscale_buffer_0').texture, 'bloomTexture', 1)
        self.vao.render()

    def render_bloom(self) -> None:
        """
        GPU downsamples and upsamples the bloom texture to blur it
        """

        n = self.bloom_depth
        self.ctx.enable(mgl.BLEND)
        self.ctx.blend_func = mgl.ADDITIVE_BLENDING
    

        self.downsample(self.framebuffer.color_attachments[1], getattr(self, f'bloom_buffer_{0}'))

        for i in range(0, n):
            self.downsample(getattr(self, f'bloom_buffer_{i}'), getattr(self, f'bloom_buffer_{i + 1}'))



        self.upsample(getattr(self, f'bloom_buffer_{n - 1}'), getattr(self, f'bloom_buffer_{n}'), getattr(self, f'upscale_buffer_{n}'))

        for i in range(n - 1, -1, -1):
            self.upsample(getattr(self, f'upscale_buffer_{i + 1}'), getattr(self, f'bloom_buffer_{i}'), getattr(self, f'upscale_buffer_{i}'))

    def downsample(self, source: Framebuffer, dest: Framebuffer) -> None:
        """
        
        """
        
        # Bind the source texture to the shader

        if isinstance(source, Framebuffer): texture = source.texture
        else: texture = source

        self.downsample_shader.bind(texture, 'screenTexture', 0)
        self.downsample_shader.write('textureSize', glm.ivec2(source.size))

        # Clear and use the destination fbo
        dest.use()
        dest.clear()

        # Render using the downsample vao (2x2 box filter)
        self.downsample_vao.render()

    def upsample(self, low: Framebuffer, high: Framebuffer, dest: Framebuffer) -> None:
        """
        
        """
        
        # Bind the source texture to the shader
        self.upsample_shader.bind(high.texture, 'highTexture', 0)
        self.upsample_shader.bind(low.texture, 'lowTexture', 1)
        self.upsample_shader.write('textureSize', glm.ivec2(low.size))

        # Clear and use the destination fbo
        dest.use()
        dest.clear()

        # Render using the upsample vao (3x3 tent filter)
        self.upsample_vao.render()


    def generate_bloom_buffers(self, n=4) -> None:
        """
        Generates n buffers for down/up sampling
        """


        size = self.framebuffer.size
        for i in range(n + 1):
            fbo = Framebuffer(self.engine, (max(size[0] // (2 ** (i)), 1), max(size[1] // (2 ** (i)), 1)))
            fbo.texture.repeat_x = False
            fbo.texture.repeat_y = False
            setattr(self, f'bloom_buffer_{i}', fbo)

            fbo = Framebuffer(self.engine, (max(size[0] // (2 ** (i)), 1), max(size[1] // (2 ** (i)), 1)))
            fbo.texture.repeat_x = False
            fbo.texture.repeat_y = False
            setattr(self, f'upscale_buffer_{i}', fbo)

    def use(self) -> None:
        """
        Uses the frame as a render target
        """
        
        self.framebuffer.use()

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

        self.framebuffer.save(destination)
    
    def clear(self):
        self.framebuffer.clear()

    def resize(self, size: tuple[int]=None) -> None:
        """
        Resize the frame to the given size. None for window size
        """

        self.framebuffer.resize()
        self.ping_pong_buffer.resize()
        self.generate_bloom_buffers()

    def __del__(self) -> None:
        """
        Releases memory used by the frame
        """
        
        if self.vbo: self.vbo.release()
        if self.vao: self.vao.release()