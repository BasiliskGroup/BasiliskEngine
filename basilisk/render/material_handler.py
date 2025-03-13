import moderngl as mgl
from ..render.image_handler import ImageHandler
from ..render.material import Material
import numpy as np


class MaterialHandler():
    engine: ...
    """Back reference to the parent engine"""
    ctx: mgl.Context
    """Back reference to the parent context"""
    materials: list[Material]
    """List containing all the materials in the engine"""
    data_texture: mgl.Texture
    """ModernGL texture containing all the material data for materials in the engine"""
    image_handler: ImageHandler=None
    """Handler for all images in the game"""
  
    def __init__(self, engine) -> None:
        """
        Handles all the materials introduced to an engine. 
        Writes material information to the GPU
        """
        
        # Back references
        self.engine = engine
        self.ctx    = engine.ctx

        # Initialize data
        self.materials = []
        self.data_texture = None

        self.image_handler = ImageHandler(engine)

    def add(self, material: Material) -> None:
        """
        Adds the given material to the handler if it is not already present
        """
        
        write = False

        if isinstance(material, Material): material = [material]

        for mtl in material:
            # Check that the material is not already in the scene
            if mtl in self.materials: continue
            # Update the material's handler
            mtl.material_handler = self
            # Add images
            if mtl.texture: self.image_handler.add(mtl.texture)
            if mtl.normal:  self.image_handler.add(mtl.normal)

            # Add the material
            self.materials.append(mtl)

            write = True

        
        # Write materials
        if write: self.write(regenerate=True)

    def generate_material_texture(self) -> None:
        """
        Generates the texture that is used to write material data to the GPU
        """

        # Check that there are materials to write
        if len(self.materials) == 0: return

        # Release existing data texture
        if self.data_texture: self.data_texture.release()

        # Create empty texture data
        material_data = np.zeros(shape=(len(self.materials), 25), dtype="f4")

        # Get data from the materials
        for i, mtl in enumerate(self.materials):
            mtl.index = i
            material_data[i] = mtl.get_data()

        # Create texture from data
        material_data = np.ravel(material_data)
        self.data_texture = self.ctx.texture((1, len(material_data)), components=1, dtype='f4', data=material_data)

    def write(self, regenerate=False) -> None:
        """
        Writes all material data to relavent shaders
        """

        if regenerate: self.generate_material_texture()

        if not self.data_texture: return

        for shader in self.engine.shader_handler.shaders:
            if 'materialsTexture' not in shader.uniforms: continue

            shader.program['materialsTexture'] = 9
            self.data_texture.use(location=9)

    def get(self, identifier: str | int) -> any:
        """
        Gets the basilisk material with the given name or index
        Args:
            identifier: str | int
                The name string or index of the desired material
        """

        # Simply use index if given
        if isinstance(identifier, int): return self.materials[identifier]

        # Else, search the list for an image material the given name
        for material in self.materials:
            if material.name != identifier: continue
            return material
        
        # No matching material found
        return None
    
    def set_base(self):
        """
        Creates a base material
        """
        
        self.base = Material('Base')
        self.materials.append(self.base)
        self.generate_material_texture()
        self.write()

    def __del__(self) -> None:
        """
        Releases the material data texture
        """

        if self.data_texture: self.data_texture.release()