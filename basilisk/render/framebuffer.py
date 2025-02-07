import moderngl as mgl
from PIL import Image



class Framebuffer:
    engine: ...
    """Reference to the parent engine"""
    fbo: mgl.Framebuffer = None
    """The core framebuffer the object provides abstraction for."""
    texture: mgl.Texture = None
    """The color texture of the framebuffer"""
    depth: mgl.Texture = None
    """The depth texture of the framebuffer"""
    size: tuple[int]
    """The dimensions of the framebuffer (x, y)"""

    def __init__(self, engine, size: tuple[int]=None, components: int=4, filter=(mgl.LINEAR, mgl.LINEAR)) -> None:
        """
        Abstraction to the mgl framebuffer
        Args:
            engine: mgl.Engine: 
                The parent engine
            size: tuple[int]: 
                The dimensions of the framebuffer (x, y)
        """
        
        # Set attributes
        self.engine = engine
        self.ctx = engine.ctx
        self.size = size if size else engine.win_size
        self.components = components

        # Create the fbo
        self.texture = self.ctx.texture(self.size, components=self.components)
        self.depth = self.ctx.depth_texture(self.size)
        self.fbo   = self.ctx.framebuffer([self.texture], self.depth)

        print()

        self.filter = filter

    def use(self) -> None:
        """
        Select this framebuffer for use
        """

        self.fbo.use()

    def clear(self) -> None:
        """
        Clear all data currently in the textures (set to black)        
        """

        self.fbo.clear()

    def save(self, destination: str=None) -> None:
        """
        Saves the frame as an image to the given file destination
        """

        path = destination if destination else 'screenshot'

        data = self.fbo.read(components=3, alignment=1)
        img = Image.frombytes('RGB', self.size, data).transpose(Image.FLIP_TOP_BOTTOM)
        img.save(f'{path}.png')

    def resize(self, size: tuple[int]=None) -> None:
        """
        Resize the buffer to the given size. None for window size
        """

        # Release old memory
        self.__del__()

        # Set/get size attribute
        self.size = size if size else self.engine.win_size

        # Create the fbo
        self.texture = self.ctx.texture(self.size, components=self.components)
        self.depth = self.ctx.depth_texture(self.size)
        self.fbo   = self.ctx.framebuffer([self.texture], self.depth)

        self.filter = self._filter

    @property
    def data(self):
        return self.fbo.read(components=3, alignment=1)

    @property
    def filter(self):
        return self.texture.filter
    
    @filter.setter
    def filter(self, value):
        self._filter = value
        self.texture.filter = value

    def __repr__(self) -> str:
        return f'<bsk.Framebuffer | size: {self.size}>' 

    def __del__(self) -> None:
        # Release any existing memory in case of a resize
        if self.texture: self.texture.release()
        if self.depth: self.depth.release()
        if self.fbo:   self.fbo.release()