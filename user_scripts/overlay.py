import moderngl as mgl
import glm
import numpy as np
from math import cos, sin, atan2


def get_line_data(p1, p2):
    p1 = glm.vec2(p1)
    p2 = glm.vec2(p2)

    width = 0.005
    unit = glm.normalize(p1 - p2) * width * 2
    theta = atan2(*unit)
    perp_vector = glm.vec2(cos(-theta), sin(-theta)) * width

    v1 = glm.vec3(p1 - perp_vector, 0.0)
    v2 = glm.vec3(p1 + perp_vector, 0.0)
    v3 = glm.vec3(p2 - perp_vector, 0.0)
    v4 = glm.vec3(p2 + perp_vector, 0.0)
    v5 = glm.vec3(p1 + unit, 0.0)
    v6 = glm.vec3(p2 - unit, 0.0)

    data = (v1, v3, v4, 
            v2, v1, v4,
            v1, v2, v5,
            v4, v3, v6)
    
    data = np.array(data, dtype='f4')

    return data

class Overlay:
    def __init__(self, scene):
        self.scene = scene
        self.vbo = self.scene.ctx.buffer(get_line_data((0, 0), (0.5, 0.5)), dynamic=True)
        program = self.scene.vao_handler.shader_handler.programs["overlay"]
        self.vao = self.scene.ctx.vertex_array(program, [(self.vbo, "3f", *["in_position"])], skip_errors=True)

    def render(self, p1: tuple=None, p2: tuple=None):
        """
        Points given in pixel coords
        """
        
        # Default p1 to center screen
        if not p1: p1 = (self.scene.engine.win_size[0] / 2, self.scene.engine.win_size[1] / 2)
        # Default p2 to the mouse position
        if not p2: p2 = self.scene.engine.mouse_position

        # Adjust coords to OpenGL grid
        width, height = self.scene.engine.win_size
        p1 = (p1[0] / width * 2 - 1, -(p1[1] / height * 2 - 1))
        p2 = (p2[0] / width * 2 - 1, -(p2[1] / height * 2 - 1))

        # Get the line poligonization data and render
        self.vbo.write(get_line_data(p1, p2))
        self.vao.render()

    def release(self):
        self.vao.release()
        self.vbo.release()