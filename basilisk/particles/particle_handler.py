from .particle_renderer import ParticleRenderer 
from ..mesh.mesh import Mesh
from ..render.material import Material
from ..generic.input_validation import validate_tuple3, validate_float

class ParticleHandler:
    def __init__(self, scene, shader=None):
        """
        A handler for all particles in a scene
        """
        
        self.scene = scene
        self.shader = shader
        self.cube = Mesh(scene.engine.root + '/bsk_assets/cube.obj')
        self.particle_renderers = {self.cube : ParticleRenderer(scene, self.cube, self.shader)}


    def add(self, mesh: Mesh=None, life: float=1.0, position: tuple|float=0, material: Material=None, scale: float=1.0, velocity: tuple|float=0, acceleration: tuple|float=0) -> bool:
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
        if mesh not in self.particle_renderers: self.particle_renderers[mesh] = ParticleRenderer(self.scene, mesh, self.shader)

        # Get material ID
        if material == None: material_index = 0
        elif isinstance(material, Material): 
            self.scene.engine.material_handler.add(material)
            material_index = material.index
        else: raise ValueError(f'particle_handler.add: Invalid particle material type: {type(material)}')

        # Validate the 3-component vectors
        position     = validate_tuple3('particle', 'add', position)
        velocity     = validate_tuple3('particle', 'add', velocity)
        acceleration = validate_tuple3('particle', 'add', acceleration)

        # Validate float inputs
        life = validate_float('particle', 'add', life)
        scale = validate_float('particle', 'add', scale)

        # Add the particle to the renderer
        self.particle_renderers[mesh].add(life, position, material_index, scale, velocity, acceleration)

    def render(self) -> None:
        for renderer in self.particle_renderers.values(): renderer.render()
    def update(self) -> None:
        for renderer in self.particle_renderers.values(): renderer.update()