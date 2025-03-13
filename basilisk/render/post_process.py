import numpy as np
import moderngl as mgl
from .shader import Shader
from .image import Image
from .framebuffer import Framebuffer


class PostProcess:
    engine: ...
    """Reference to the parent engine"""
    ctx: mgl.Context
    """Reference to the parent context"""
    shader: Shader
    """Shader object used by the post process"""
    vao: mgl.VertexArray
    """Screenspace render vao"""

    def __init__(self, engine, shader_path: str=None, size: tuple=None, components: int=4, filter=(mgl.LINEAR, mgl.LINEAR)) -> None:
        """
        Object to apply post processing to a texture
        """
        
        # Reference attributes
        self.engine = engine
        self.ctx = engine.ctx

        # Size of the destination
        self.size = size if size else engine.win_size
        self.components = components

        # Load default fragment if none given
        if not (isinstance(shader_path, str) or isinstance(shader_path, type(None))): 
            raise ValueError(f'PostProces.apply: Invalid post process source type {type(shader_path)}. Expected string path destination of fragment shader.')
        frag = shader_path if shader_path else self.engine.root + f'/shaders/frame.frag'

        # Load Shaders
        self.shader = Shader(self.engine, self.engine.root + f'/shaders/frame.vert', frag)
        self.engine.scene.shader_handler.add(self.shader)

        # Load VAO
        self.vbo = self.ctx.buffer(np.array([[-1, -1, 0, 0, 0], [1, -1, 0, 1, 0], [1, 1, 0, 1, 1], [-1, 1, 0, 0, 1], [-1, -1, 0, 0, 0], [1, 1, 0, 1, 1]], dtype='f4'))
        self.vao = self.ctx.vertex_array(self.shader.program, [(self.vbo, '3f 2f', 'in_position', 'in_uv')], skip_errors=True)

        # Temporary render destination
        self.fbo = Framebuffer(engine, self.size, components)

        # Filter settings
        self.filter = filter
        self.fbo.texture.filter = self.filter


    def apply(self, source: mgl.Texture | Image | Framebuffer, destination: mgl.Texture | Image | Framebuffer=None) -> mgl.Texture | Image | Framebuffer:
        """
        Applies a post process shader to a texture source.
        Returns the modified texture or renders to the destination if given
        """

        if isinstance(source, Framebuffer):   return self._apply_to_framebuffer(source, destination)
        elif isinstance(source, mgl.Texture): return self._apply_to_texture(source, destination)
        elif isinstance(source, Image):       return self._apply_to_image(source, destination)
        
        raise ValueError(f'PostProces.apply: Invalid postprocess source type {type(source)}')

    def resize(self, size: tuple=None):
        """
        Resize the post process
        """
        
        self.size = size if size else self.engine.win_size

    def _render_post_process(self, source: mgl.Texture):
        # Clear and use the fbo as a render target
        self.fbo.use()
        self.fbo.clear()

        # Load the source texture to the shader
        self.shader.program['screenTexture'] = 0
        source.use(location=0)

        # Apply the post process
        self.vao.render()


    def _apply_to_framebuffer(self, source: Framebuffer, detination: Framebuffer=None) -> Framebuffer:
        """
        Applies a post process to a bsk.Framebuffer
        """

        # Create a blank framebuffer
        if not detination or detination.size != self.size:
            fbo = Framebuffer(self.engine, self.size, self.components)
            old_filter = None
            fbo.texture.filter = self.filter
        else:
            fbo = detination
            old_filter = fbo.filter
            fbo.texture.filter = self.filter

        # Load the source texture to the shader
        self.shader.program['screenTexture'] = 0
        source.texture.use(location=0)

        fbo.use()
        fbo.clear()
        # Apply the post process
        self.vao.render()

        # Reset filter if needed
        if old_filter: fbo.texture.filter = old_filter

        # Return the fbo for the user
        return fbo

    def _apply_to_texture(self, source: mgl.Texture, destination: mgl.Texture) -> mgl.Texture:
        """
        Applies a post process to a mgl.Texture
        """

        # Render the post processs with the given texture
        self._render_post_process(source)

        # Make a deep copy of the modified texture
        texture = self.ctx.texture(self.fbo.size, self.fbo.components, self.fbo.data)

        return texture
    
    def _apply_to_image(self, source: Image, destination: Image) -> Image:
        """
        Applies a post process to a bsk.Image
        """

        # Create a texture from the image data
        texture = self.ctx.texture(self.fbo.size, self.fbo.components, source.data)

        # Render the post processs with the given texture
        self._render_post_process(source)

        # Make an image from the texture
        image = Image()
        return image