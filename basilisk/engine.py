import os
from sys import platform
os.environ['PYGAME_HIDE_SUPPORT_PROMPT'] = "hide"
import pygame as pg
import moderngl as mgl
import glcontext  # For packaging (so it isnt a hidden import)
from .render.shader_handler import ShaderHandler
from .render.material_handler import MaterialHandler
from .render.frame import Frame
from .draw.draw_handler import DrawHandler
from .config import Config
from .input_output.mouse import Mouse
from .input_output.clock import Clock
from .input_output.IO_handler import IO
from .mesh.cube import Cube

class Engine():
    win_size: tuple
    """Size of the engine window in pixels"""
    ctx: mgl.Context
    """ModernGL context used by the engine"""
    clock: Clock
    """Basilisk clock used to keep track of time"""
    shader_handler: ShaderHandler=None
    """Handler for all shaders used in all scenes of the engine"""
    material_handler: MaterialHandler=None
    """Handler for all materials and images in all scenes"""
    frame: Frame=None
    """Default render target for all locations. Rendered to the screen at the end of the frame"""
    config: Config
    """Object containing all global attributes"""
    delta_time: float
    """Time in seconds that passed between the last frame"""
    time: float
    """Total time the engine has been running"""
    running: bool=True
    """True if the engine is still running"""
    mouse: Mouse
    """Object containing information about the user's mouse"""
    root: str
    """Path to the root directory containing internal data"""
    current_frame_updated: bool=False
    """Flag for if the engine has been updated this frame"""
    keys: list[bool]
    """List of all keyboard inputs as booleans"""
    previous_keys: list[bool]
    """List of all keyoard inputs from the last frame as booleans"""

    def __init__(self, win_size=(800, 800), title="Basilisk Engine", vsync=None, max_fps=None, grab_mouse=True, headless=False, resizable=True) -> None:
        """
        Basilisk Engine Class. Sets up the engine enviornment and allows the user to interact with Basilisk
        Args:
            win_size: tuple
                The initial window size of the engine
            title: str
                The title of the engine window
            vsync: bool
                Flag for running engine with vsync enabled
            headless: bool
                Flag for headless rendering
        """

        # Save the window size
        self.win_size = win_size

        # Initialize pygame and set OpenGL attributes
        pg.init()  
        pg.display.gl_set_attribute(pg.GL_CONTEXT_MAJOR_VERSION, 3)
        pg.display.gl_set_attribute(pg.GL_CONTEXT_MINOR_VERSION, 3)
        pg.display.gl_set_attribute(pg.GL_CONTEXT_PROFILE_MASK, pg.GL_CONTEXT_PROFILE_CORE)

        # Platform settings
        if platform == 'win32' : self.platform = 'windows'
        elif  platform == 'darwin': self.platform = 'mac' 
        else: self.platform = 'linux'
        if vsync == None: vsync = True if self.platform == 'linux' else False

        # Initializae the pygame display
        self.headless = headless
        if headless:
            pg.display.set_mode((300, 50), vsync=vsync, flags=pg.OPENGL | pg.DOUBLEBUF)
            pg.display.iconify()
        else:
            if resizable: pg.display.set_mode(self.win_size, vsync=vsync, flags=pg.OPENGL | pg.DOUBLEBUF | pg.RESIZABLE)
            else: pg.display.set_mode(self.win_size, vsync=vsync, flags=pg.OPENGL | pg.DOUBLEBUF)
                
        # Initalize pygame sound moduel sound
        pg.mixer.pre_init(44100, -16, 2, 512)
        pg.mixer.init()
        pg.mixer.set_num_channels(64)
        pg.mixer.music.set_volume(100/100)

        # MGL context setup
        self.ctx = mgl.create_context()
        self.ctx.enable(flags=mgl.DEPTH_TEST | mgl.CULL_FACE | mgl.BLEND)

        # Global attributes referenced by the handlers
        self.config = Config()
        self.root = os.path.dirname(__file__)
        self.cube = Cube(self)
        self.fbos = []
        
        # Handlers
        self.clock            = Clock(self, max_fps)
        self.IO               = IO(self, grab_mouse=grab_mouse, caption=title)
        self.material_handler = MaterialHandler(self)
        self.shader_handler   = ShaderHandler(self)
        self.draw_handler     = DrawHandler(self)
        self.frame            = Frame(self)
        self.material_handler.set_base()

    def _update(self) -> None:
        """
        Internal engine update.
        Updates all input, physics, and time variables. Clears the frame.
        """

        if self.current_frame_updated: return

        for fbo in self.fbos: fbo.clear()
        self.clock.update()
        self.IO.update()

        self.current_frame_updated = True


    def update(self) -> None:
        """
        Calls internal update if needed
        Renders the draw handler
        Renders the engine's frame to the screen.
        """


        # Must update the frame
        if not self.current_frame_updated: self._update()
        if not self.running: return

        # Render all draw calls from the past frame
        self.frame.use()
        self.draw_handler.render()

        # Clear the screen and render the frame
        self.ctx.screen.use()
        self.ctx.clear()
        self.frame.render()
        pg.display.flip()

        self.frame.clear()


        # Allow for the engine to take in input again
        self.current_frame_updated = False

    def quit(self) -> None:
        """
        Stops the engine and releases all memory
        """

        pg.quit()
        self.ctx.release()
        self.running = False


    @property
    def shader(self): return self._shader