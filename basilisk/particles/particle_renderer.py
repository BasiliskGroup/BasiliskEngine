import numpy as np
from ..render.shader import Shader
from ..mesh.mesh import Mesh
from ..render.material import Material
from numba import njit


@njit
def update_particle_matrix(particle_instances, dt):
    particle_instances[:,6:9] += particle_instances[:,9:12] * dt
    particle_instances[:,:3] += particle_instances[:,6:9] * dt
    particle_instances[:,5] -= dt/3
    return particle_instances

@njit
def get_alive(particles):
    return particles[particles[:, 5] >= 0]

update_particle_matrix(np.zeros(shape=(2, 12), dtype='f4'), 1)
get_alive(np.zeros(shape=(2, 12), dtype='f4'))


class ParticleRenderer:
    def __init__(self, scene: ..., mesh: Mesh, shader: Shader=None) -> None:
        """
        Handels and renders the particles of a single mesh type
        """
        
        self.scene = scene
        self.ctx = scene.ctx
        root = scene.engine.root
        if shader: self.shader = shader
        else: self.shader = Shader(scene.engine, vert=root + '/shaders/particle.vert', frag=root + '/shaders/particle.frag')
        scene.engine.shader_handler.add(self.shader)

        self.particle_cube_size = 25

        self.particle_instances = np.zeros(shape=(1, 12), dtype='f4')
        self.instance_buffer = self.ctx.buffer(reserve=(12 * 3) * (self.particle_cube_size ** 3))
        
        self.vao = self.ctx.vertex_array( self.shader.program, 
                                        [(self.ctx.buffer(mesh.data), '3f 2f 3f 3f 3f', *['in_position', 'in_uv', 'in_normal', 'in_tangent', 'in_bitangent']), 
                                         (self.instance_buffer, '3f 1f 1f 1f /i', 'in_instance_pos', 'in_instance_mtl', 'scale', 'life')], 
                                          skip_errors=True)

    def render(self) -> None:
        """
        Renders the alive particles in the scene
        """
        
        # Get the current particles
        alive_particles = get_alive(self.particle_instances)
        n = len(alive_particles)

        # Write and render
        self.instance_buffer.write(np.array(alive_particles[:,:6], order='C'))
        self.vao.render(instances=n)

    def update(self) -> None:
        """
        Updates the particle positions based on their given properties
        """
        
        self.particle_instances = get_alive(self.particle_instances)
        self.particle_instances = update_particle_matrix(self.particle_instances, self.scene.engine.delta_time)

    def add(self, life=1.0, position=(0, 0, 0), material: int=0, scale=1.0, velocity=(0, 3, 0), acceleration=(0, -10, 0)) -> bool:
        """
        Add a new particle to the scene
        Args:
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
        # Check if there is already the max number of particles
        if len(self.particle_instances) >= (self.particle_cube_size ** 3): return False
        # Create and add the particle to the scene
        new_particle = np.array([*position, material, scale, life, *velocity, *acceleration])
        self.particle_instances = np.vstack([new_particle, self.particle_instances], dtype='f4')


    def __del__(self):
        self.instance_buffer.release()
        self.vao.release()