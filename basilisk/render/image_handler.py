import moderngl as mgl
import glm
import numpy as np

texture_sizes = (8, 64, 512, 1024, 2048)


class ImageHandler():
    engine: ...
    """Back refernce to the parent engine"""
    ctx: mgl.Context
    """Back reference to the Context used by the scene/engine"""
    images: list
    """List of basilisk Images containing all the loaded images given to the scene"""
    texture_arrays: dict
    """Dictionary of textures arrays for writting textures to GPU"""

    def __init__(self, engine) -> None:
        """
        Container for all the basilisk image objects in the scene.
        Handles the managment and writting of all image textures.
        """
        
        # Set back references
        self.engine = engine
        self.ctx    = engine.ctx

        self.images = []
        self.texture_arrays = {size : [] for size in texture_sizes}

    def add(self, image: any) -> None:
        """
        Adds an existing basilisk image object to the handler for writting
        Args:
            image: bsk.Image
                The existing image that is to be added to the scene.
        """
        
        if image in self.images: return

        self.images.append(image)
        self.write(regenerate=True)

    def generate_texture_array(self) -> None:
        """
        Generates texutre arrays for all the images. Updates the index of the image instance
        """

        # Release any existsing texture arrays
        for texture_array in self.texture_arrays.values():
            if not texture_array: continue
            texture_array.release()

        self.texture_arrays = {size : [] for size in texture_sizes}

        for image in self.images:
            # Add the image data to the array
            self.texture_arrays[image.size].append(image.data)
            # Update the image index
            image.index = glm.ivec2(texture_sizes.index(image.size), len(self.texture_arrays[image.size]) - 1)


        for size in self.texture_arrays:
            # Check that there are textures in the bucket
            if not len(self.texture_arrays[size]): continue
            array_data = np.array(self.texture_arrays[size])
            dim = (size, size, len(self.texture_arrays[size]))
            
            # Make the array
            self.texture_arrays[size] = self.ctx.texture_array(size=dim, components=4, data=array_data)
            # Texture OpenGl settings
            self.texture_arrays[size].build_mipmaps()
            if size > 32: self.texture_arrays[size].filter = (mgl.LINEAR_MIPMAP_LINEAR, mgl.LINEAR)
            else: self.texture_arrays[size].filter = (mgl.NEAREST, mgl.NEAREST)
            self.texture_arrays[size].anisotropy = 32.0

    def write(self, regenerate=False) -> None:
        """
        Writes all texture arrays to shaders that use images 
        """

        if not self.engine.shader_handler: return

        if regenerate: self.generate_texture_array()

        if not self.texture_arrays: return

        for shader in self.engine.shader_handler.shaders:
            if 'textureArrays[5]' not in shader.uniforms: continue

            for i, size in enumerate(texture_sizes):
                if not size in self.texture_arrays: continue
                if not self.texture_arrays[size]: continue
                shader.program[f'textureArrays[{i}].array'] = i + 3
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
        
        # [texture_array.release() for texture_array in self.texture_arrays]