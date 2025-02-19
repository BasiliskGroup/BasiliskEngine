import moderngl as mgl
import glm
from ..render.light import DirectionalLight


class LightHandler():
    engine: ...
    """Back reference to the parent engine"""
    scene: ...
    """Back reference to the parent scene"""
    ctx: mgl.Context
    """Back reference to the parent context"""
    directional_light: DirectionalLight
    """The directional light of the scene"""
    point_lights: list
    """List of all the point lights in the scene"""

    def __init__(self, scene) -> None:
        """
        Handles all the lights in a Basilisk scene.
        """

        # Back references
        self.scene  = scene
        self.engine = scene.engine
        self.ctx    = scene.engine.ctx

        # Intialize light variables
        self.directional_lights = None
        self.directional_lights = [DirectionalLight(self, direction=dir, intensity=intensity) for dir, intensity in zip(((1, -1, 1), (-.1, 3, -.1)), (1, .05))]
        self.point_lights       = []

        # Initalize uniforms
        self.write()

    def write(self, program: mgl.Program=None, directional=True, point=False) -> None:
        """
        Writes all the lights in a scene to the given shader program
        """

        # if not program: program = self.engine.shader.program

        for shader in self.engine.shader_handler.shaders:
            if 'numDirLights' not in shader.uniforms: continue
            
            program = shader.program

            if directional and self.directional_lights and 'numDirLights' in self.engine.shader.uniforms:

                program['numDirLights'].write(glm.int32(len(self.directional_lights)))

                for i, light in enumerate(self.directional_lights):
                    program[f'dirLights[{i}].direction'].write(light.direction)
                    program[f'dirLights[{i}].intensity'].write(glm.float32(light.intensity))
                    program[f'dirLights[{i}].color'    ].write(light.color / 255.0)
                    program[f'dirLights[{i}].ambient'  ].write(glm.float32(light.ambient))
            
            if point:
                ...