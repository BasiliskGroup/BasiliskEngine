import moderngl as mgl
import glm
import numpy as np
from PIL import Image as PIL_Image


texture_sizes = (128, 256, 512, 1024, 2048)


class Image():
    name: str
    """Name of the image"""   
    index: glm.ivec2
    """Location of the image in the texture arrays"""
    data: np.ndarray
    """Array of the texture data"""
    size: int
    """The width and height in pixels of the image"""

    def __init__(self, path: str) -> None:
        """
        A basilisk image object that contains a moderngl texture
        Args:
            path: str
                The string path to the image
        """
        
        # Get name from path
        self.name = path.split('/')[-1].split('\\')[-1].split('.')[0]

        # Set the texture
        # Load image
        img = PIL_Image.open(path).convert('RGBA')
        # Set the size in one of the size buckets
        size_buckets = texture_sizes
        self.size = size_buckets[np.argmin(np.array([abs(size - img.size[0]) for size in size_buckets]))]
        img = img.resize((self.size, self.size)) 
        # Get the image data
        self.data = img.tobytes()

        # Default index value (to be set by image handler)
        self.index = glm.ivec2(1, 1)

    def __repr__(self) -> str:
        """
        Returns a string representation of the object
        """
        
        return f'<Basilisk Image | {self.name}, ({self.size}x{self.size})>'