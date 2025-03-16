import moderngl as mgl
import glm
from .framebuffer import Framebuffer
from .shader import Shader


class Bloom:
    def __init__(self, frame):
        self.engine = frame.engine
        self.ctx = self.engine.ctx
        self.frame = frame

        # Load bloom tools
        self.downsample_shader = Shader(self.engine, self.engine.root + '/shaders/frame.vert', self.engine.root + '/shaders/bloom_downsample.frag')
        self.upsample_shader   = Shader(self.engine, self.engine.root + '/shaders/frame.vert', self.engine.root + '/shaders/bloom_upsample.frag')
        
        self.engine.shader_handler.add(self.downsample_shader)
        self.engine.shader_handler.add(self.upsample_shader)

        self.downsample_vao = self.ctx.vertex_array(self.downsample_shader.program, [(self.frame.vbo, '3f 2f', 'in_position', 'in_uv')], skip_errors=True)
        self.upsample_vao   = self.ctx.vertex_array(self.upsample_shader.program, [(self.frame.vbo, '3f 2f', 'in_position', 'in_uv')], skip_errors=True)

        self.generate_bloom_buffers()

    def render(self) -> None:
        """
        GPU downsamples and upsamples the bloom texture to blur it
        """
    
        if not self.frame.engine.config.bloom_enabled:
            getattr(self, f'upscale_buffer_0').clear()
            return

        n = self.engine.config.bloom_quality

        self.downsample(self.frame.framebuffer.color_attachments[1], getattr(self, f'bloom_buffer_{0}'))
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

    def generate_bloom_buffers(self) -> None:
        """
        Generates n buffers for down/up sampling
        """

        n = self.engine.config.bloom_quality
        size = self.frame.framebuffer.size

        for i in range(100):
            try:
                getattr(self, f'bloom_buffer_{i}').__del__()
                getattr(self, f'upscale_buffer_{i}').__del__()
            except:
                ...

        for i in range(n + 1):
            fbo = Framebuffer(self.engine, (max(size[0] // (2 ** (i)), 1), max(size[1] // (2 ** (i)), 1)))
            fbo.texture.repeat_x = False
            fbo.texture.repeat_y = False
            setattr(self, f'bloom_buffer_{i}', fbo)

            fbo = Framebuffer(self.engine, (max(size[0] // (2 ** (i)), 1), max(size[1] // (2 ** (i)), 1)))
            fbo.texture.repeat_x = False
            fbo.texture.repeat_y = False
            setattr(self, f'upscale_buffer_{i}', fbo)

    @property
    def texture(self): return getattr(self, f'upscale_buffer_0').texture