import moderngl as mgl
from ..render.image_handler import ImageHandler
from ..render.material import Material
import numpy as np


class MaterialHandler():
    engine: ...
    """Back reference to the parent engine"""
    scene: ...
    """Back reference to the parent scene"""
    ctx: mgl.Context
    """Back reference to the parent context"""
    materials: list[Material]
    """List containing all the materials in the scene"""
    data_texture: mgl.Texture
    """ModernGL texture containing all the material data for materials in the scene"""
  
    def __init__(self, scene) -> None:
        """
        Handles all the materials introduced to a scene. 
        Writes material information to the GPU
        """
        
        # Back references
        self.scene  = scene
        self.engine = scene.engine
        self.ctx    = scene.engine.ctx

        # Initialize data
        self.materials = []
        self.data_texture = None
        self.set_base()

        self.image_handler = ImageHandler(scene)

    def add(self, material: Material) -> None:
        """
        Adds the given material to the handler if it is not already present
        """
        
        # Check that the material is not already in the scene
        if material in self.materials: return None
        # Update the material's handler
        material.material_handler = self
        # Add images
        if material.texture: self.image_handler.add(material.texture)
        if material.normal:  self.image_handler.add(material.normal)

        # Add the material
        self.materials.append(material)
        # Write materials
        self.write()

    def generate_material_texture(self) -> None:
        """
        Generates the texture that is used to write material data to the GPU
        """

        # Check that there are materials to write
        if len(self.materials) == 0: return

        # Release existing data texture
        if self.data_texture: self.data_texture.release()

        # Create empty texture data
        material_data = np.zeros(shape=(len(self.materials), 17), dtype="f4")

        # Get data from the materials
        for i, mtl in enumerate(self.materials):
            mtl.index = i
            material_data[i] = mtl.get_data()

        # Create texture from data
        material_data = np.ravel(material_data)
        self.data_texture = self.ctx.texture((1, len(material_data)), components=1, dtype='f4', data=material_data)

    def write(self, shader_program: mgl.Program=None) -> None:
        """
        Writes all material data to the given shader
        """

        if shader_program == None: shader_program = self.scene.shader_handler.programs['batch']

        self.generate_material_texture()

        shader_program[f'materialsTexture'] = 9
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

    def __del__(self) -> None:
        """
        Releases the material data texture
        """

        if self.data_texture: self.data_texture.release()