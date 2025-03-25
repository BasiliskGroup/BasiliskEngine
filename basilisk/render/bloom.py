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
        
        self.downsample_vao = self.ctx.vertex_array(self.downsample_shader.program, [(self.frame.vbo, '3f 2f', 'in_position', 'in_uv')], skip_errors=True)
        self.upsample_vao   = self.ctx.vertex_array(self.upsample_shader.program, [(self.frame.vbo, '3f 2f', 'in_position', 'in_uv')], skip_errors=True)

        self.upsample_buffers = []
        self.downsample_buffers = []
        self.generate_bloom_buffers()

    def render(self) -> None:
        """
        GPU downsamples and upsamples the bloom texture to blur it
        """
    
        self.clear()

        n = self.engine.config.bloom_quality


        self.ctx.enable(mgl.BLEND)
        self.ctx.blend_func = mgl.ADDITIVE_BLENDING

        # Render the screen's bloom texture to a local buffer
        bloom_texture = self.frame.input_buffer.color_attachments[1]
        self.downsample_shader.bind(bloom_texture, 'screenTexture', 0)
        self.downsample_shader.write(glm.ivec2(bloom_texture.size), 'textureSize')
        self.downsample_buffers[0].clear()
        self.downsample_buffers[0].use()
        self.downsample_vao.render()

        for i in range(0, n):
            self.downsample(self.downsample_buffers[i], self.downsample_buffers[i + 1])

        self.upsample(self.downsample_buffers[n - 1], self.downsample_buffers[n], self.upsample_buffers[n])
        for i in range(n - 1, -1, -1):
            self.upsample(self.upsample_buffers[i + 1], self.downsample_buffers[i], self.upsample_buffers[i])

        self.ctx.disable(mgl.BLEND)

    def downsample(self, source: Framebuffer, dest: Framebuffer) -> None:
        """
        
        """
        
        # Bind the source texture to the shader

        if isinstance(source, Framebuffer): texture = source.texture
        else: texture = source

        self.downsample_shader.bind(texture, 'screenTexture', 0)
        self.downsample_shader.write(glm.ivec2(source.size), 'textureSize')

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
        self.upsample_shader.write(glm.ivec2(low.size), 'textureSize')

        # Clear and use the destination fbo
        dest.use()
        dest.clear()

        # Render using the upsample vao (3x3 tent filter)
        self.upsample_vao.render()

        return dest

    def generate_bloom_buffers(self) -> None:
        """
        Generates n buffers for down/up sampling
        """

        n = self.engine.config.bloom_quality
        size = self.frame.input_buffer.size

        self.downsample_buffers = []
        self.upsample_buffers = []

        for i in range(n + 1):
            downsample_fbo = Framebuffer(self.engine, size=(max(size[0] // (2 ** (i)), 1), max(size[1] // (2 ** (i)), 1)))
            upsample_fbo = Framebuffer(self.engine, size=(max(size[0] // (2 ** (i)), 1), max(size[1] // (2 ** (i)), 1)))

            self.downsample_buffers.append(downsample_fbo)
            self.upsample_buffers.append(upsample_fbo)

    def clear(self):
        for buffer in self.upsample_buffers + self.downsample_buffers:
            buffer.clear()

    @property
    def fbo(self): return self.upsample_buffers[0]
    @property
    def texture(self): return self.fbo.texture