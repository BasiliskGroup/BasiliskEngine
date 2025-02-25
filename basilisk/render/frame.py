import numpy as np
import moderngl as mgl
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
        self.framebuffer = Framebuffer(self.engine, scale=scale, linear_filter=linear_filter)
        self.ping_pong_buffer = Framebuffer(self.engine, scale=scale, linear_filter=linear_filter)

        # Load Shaders
        self.shader = Shader(self.engine, self.engine.root + '/shaders/frame.vert', self.engine.root + '/shaders/frame.frag')
        self.engine.shader_handler.add(self.shader)

        # Load VAO
        self.vbo = self.ctx.buffer(np.array([[-1, -1, 0, 0, 0], [1, -1, 0, 1, 0], [1, 1, 0, 1, 1], [-1, 1, 0, 0, 1], [-1, -1, 0, 0, 0], [1, 1, 0, 1, 1]], dtype='f4'))
        self.vao = self.ctx.vertex_array(self.shader.program, [(self.vbo, '3f 2f', 'in_position', 'in_uv')], skip_errors=True)

        # TEMP TESTING
        self.post_processes = []


    def render(self) -> None:
        """
        Renders the current frame to the screen
        """

        for process in self.post_processes:
            self.ping_pong_buffer = process.apply(self.framebuffer, self.ping_pong_buffer)
            
            temp = self.framebuffer
            self.framebuffer = self.ping_pong_buffer
            self.ping_pong_buffer = temp
        
        self.ctx.screen.use()
        self.shader.program['screenTexture'] = 0
        self.framebuffer.texture.use(location=0)
        self.vao.render()


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

    def __del__(self) -> None:
        """
        Releases memory used by the frame
        """
        
        if self.vbo: self.vbo.release()
        if self.vao: self.vao.release()