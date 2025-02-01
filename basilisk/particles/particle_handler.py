from .particle_renderer import ParticleRenderer 
from ..mesh.mesh import Mesh
from ..render.material import Material


class ParticleHandler:
    def __init__(self, scene):
        """
        A handler for all particles in a scene
        """
        
        self.scene = scene
        self.cube = Mesh(scene.engine.root + '/bsk_assets/cube.obj')
        self.particle_renderers = {self.cube : ParticleRenderer(scene, self.cube)}


    def add(self, mesh: Mesh=None, life=1.0, position=(0, 0, 0), material: Material=None, scale=1.0, velocity=(0, 3, 0), acceleration=(0, -10, 0)) -> bool:
        """
        Add a new particle to the scene
        Args:
            mesh: Mesh
                The basilisk mesh of the particle
            life: float
                The duration of the particle in seconds
            position: tuple (x, y, z)
                The initial position of the particle
            color: tuple (r, g, b) (components out of 255) 
                The color of the particle
            scale: float
                The overall scale factor of the particle
            velocity: tuple (x, y, z)
                The inital velocity of the particle as a vector
            acceleration: tuple (x, y, z)
                The permanent acceleration of the particle as a vector
        """

        # Get the mesh and make a new particle renderer if the mesh is new
        if mesh == None: mesh = self.cube
        elif not isinstance(mesh, Mesh): raise ValueError(f'particle_handler.add: invlaid mesh type for particle: {type(mesh)}')
        if mesh not in self.particle_renderers: self.particle_renderers[mesh] = ParticleRenderer(self.scene, mesh)

        # Get material ID
        if material == None: material_index = 0
        elif isinstance(material, Material): 
            self.scene.material_handler.add(material)
            material_index = material.index
        else: raise ValueError(f'particle_handler.add: Invalid particle material type: {type(material)}')

        # Add the particle to the renderer
        self.particle_renderers[mesh].add(life, position, material_index, scale, velocity, acceleration)

    def render(self) -> None:
        for renderer in self.particle_renderers.values(): renderer.render()
    def update(self) -> None:
        for renderer in self.particle_renderers.values(): renderer.update()