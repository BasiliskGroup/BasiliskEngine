import moderngl as mgl
import glm
from PIL import Image as PIL_Image


class Image():
    name: str
    """Name of the image"""   
    index: glm.ivec2
    """Location of the image in the texture arrays"""
    texture: mgl.Texture
    """Moderngl texture of the image"""

    def __init__(self, engine, path: str) -> None:
        """
        A basilisk image object that contains a moderngl texture
        Args:
            engine: bsk.Engine
                The engine that the image should be part of
            path: str
                The string path to the image
        """
        
        # Get name from path
        self.name = path.split('/')[-1].split('\\')[-1].split('.')[0]

        # Set the texture
        img = PIL_Image.open(path).convert('RGBA')
        self.texture = engine.ctx.texture(img.size, components=4, data=img.tobytes())

        # Default index value (to be set by image handler)
        self.index = glm.ivec2(1, 1)

    def __repr__(self) -> str:
        """
        Returns a string representation of the object
        """
        
        return f'<Basilisk Image | {self.name}, {self.texture}>'

    def __del__(self) -> None:
        """
        Releases texture data
        """

        self.texture.release()