import moderngl as mgl
import glm
import numpy as np


class ImageHandler():
    engine: any
    """Back refernce to the parent engine"""
    scene: any
    """Back refernce to the parent scene"""
    ctx: mgl.Context
    """Back reference to the Context used by the scene/engine"""
    images: list
    """List of basilisk Images containing all the loaded images given to the scene"""
    texture_arrays: dict
    """Dictionary of textures arrays for writting textures to GPU"""

    def __init__(self, scene) -> None:
        """
        Container for all the basilisk image objects in the scene.
        Handles the managment and writting of all image textures.
        """
        
        # Set back references
        self.scene  = scene
        self.engine = scene.engine
        self.ctx    = scene.engine.ctx

        self.images = []

    def add(self, image: any) -> None:
        """
        Adds an existing basilisk image object to the handler for writting
        Args:
            image: bsk.Image
                The existing image that is to be added to the scene.
        """
        
        if image not in self.images:
            self.images.append(image)

    def generaate_texture_array(self) -> None:
        """
        Generates texutre arrays for all the images. Updates the index of the image instance
        """

        # Release any existsing texture arrays
        for texture_array in self.texture_arrays.values():
            texture_array.release()

        self.texture_arrays = {size : [] for size in self.engine.config.texture_sizes}

        for image in self.images:
            # Add the image data to the array
            self.texture_arrays[image.size].append(image.data)
            # Update the image index
            image.index = glm.ivec2(self.engine.config.texture_sizes.index(image.size), len(self.texture_arrays[image.size]) - 1)

        for size in self.texture_arrays:
            # Get the rray data and attributes
            array_data = np.array(self.texture_arrays[size], dtype='f4')
            dim = (size, size, len(self.texture_arrays[size]))

            # Make the array
            self.texture_arrays[size] = self.ctx.texture_array(size=dim, components=4, data=array_data)
            # Texture OpenGl settings
            self.texture_arrays[size].build_mipmaps()
            self.texture_arrays[size].filter = (mgl.LINEAR_MIPMAP_LINEAR, mgl.LINEAR)
            self.texture_arrays[size].anisotropy = 32.0

    def write(self, shader_program: mgl.Program) -> None:
        """
        Writes all texture arrays to the given shader program
        Args:
            shader_program: mgl.Program:
                Destination of the texture array write
        """

        for i, size in enumerate(self.engine.config.texture_sizes):
            if not size in self.texture_arrays: continue
            shader_program[f'textureArrays[{i}].array'] = i + 3
            self.texture_arrays[size].use(location=i+3)

    def get(self, identifier: str | int) -> any:
        """
        Gets the basilisk image with the given name or index
        Args:
            identifier: str | int
                The name string or index of the desired image
        """

        # Simply use index if given
        if isinstance(identifier, int): return self.images[identifier]

        # Else, search the list for an image with the given name
        for image in self.images:
            if image.name != identifier: continue
            return image
        
        # No matching image found
        return None
    
    def __del__(self):
        """
        Deallocates all texture arrays
        """
        
        [texture_array.release() for texture_array in self.texture_arrays]