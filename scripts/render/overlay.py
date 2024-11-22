import moderngl as mgl
import glm
import numpy as np
from math import cos, sin, atan2, pi

class Overlay:
    def __init__(self, scene):
        # Scene reference
        self.scene = scene
        # Array of vertex data before render
        self.data = []
        # Render objects
        self.vbo = self.scene.ctx.buffer(reserve=100_000, dynamic=True)
        program = self.scene.vao_handler.shader_handler.programs["overlay"]
        self.vao = self.scene.ctx.vertex_array(program, [(self.vbo, "2f 3f", *["in_position", "in_color"])], skip_errors=True)
    
    def render(self):
        """
        Renders the current frame of data. Flushed data after finished.
        """
        
        data = np.array(self.data, dtype="f4")

        self.vbo.write(data)
        self.vao.render()

        self.vbo.clear()
        self.data = []

    def draw_circle(self, x: float, y: float, radius: float, color: tuple=(255, 255, 255), win_size: tuple=(1, 1)):
        """
        Draws a rect between centered on x, y with width and height
            Args:
                x: float
                    Horizontal center of the rect. Given in OpenGL Screen space
                y: float
                    Vertical center of the rect. Given in OpenGL Screen space
                radius: float
                    Radius of the circle. Given in OpenGL Screen space
                color: tuple=(r, g, b)
                    Color of the line
        """

        v1 = (x, y, *color)
        theta = 0
        resolution = 25
        delta_theta = (2 * pi) / resolution

        aspect_ratio = (win_size[1] / win_size[0])

        for triangle in range(resolution):
            v2 = (x + radius * cos(theta) * aspect_ratio, y + radius * sin(theta), *color)
            theta += delta_theta
            v3 = (x + radius * cos(theta) * aspect_ratio, y + radius * sin(theta), *color)
            self.data.extend([v1, v3, v2])

    def draw_rect(self, x: float, y: float, width: float, height: float, color: tuple=(255, 255, 255)):
        """
        Draws a rect between centered on x, y with width and height
            Args:
                x: float
                    Horizontal center of the rect. Given in OpenGL Screen space
                y: float
                    Vertical center of the rect. Given in OpenGL Screen space
                width: float
                    Total width of the rectangle. Given in OpenGL Screen space
                height: float
                    Total height of the rectangle. Given in OpenGL Screen space
                color: tuple=(r, g, b)
                    Color of the line
        """

        v1 = (x - width / 2, y - height / 2, *color)
        v2 = (x + width / 2, y - height / 2, *color)
        v3 = (x - width / 2, y + height / 2, *color)
        v4 = (x + width / 2, y + height / 2, *color)

        self.data.extend([v1, v2, v3, v2, v4, v3])

    def draw_line(self, p1: tuple, p2: tuple, thickness: float=.001, color: tuple=(255, 255, 255)):
        """
        Draws a line between two points
            Args:
                p1: tuple=((x1, y1), (x2, y2))
                    Starting point of the line. Given in OpenGL Screen space
                p1: tuple=((x1, y1), (x2, y2))
                    Starting point of the line. Given in OpenGL Screen space
                thickness: float
                    Size of the line on either side. Given in OpenGL screen space (basically a percentage)
                color: tuple=(r, g, b)
                    Color of the line
        """
        
        p1 = glm.vec2(p1)
        p2 = glm.vec2(p2)

        unit = glm.normalize(p1 - p2) * thickness * 2
        theta = atan2(*unit)
        perp_vector = glm.vec2(cos(-theta), sin(-theta)) * thickness

        v1 = (*(p1 - perp_vector), *color)
        v2 = (*(p1 + perp_vector), *color)
        v3 = (*(p2 - perp_vector), *color)
        v4 = (*(p2 + perp_vector), *color)
        
        self.data.extend([v1, v4, v3, v2, v4, v1])


    def release(self):
        self.vao.release()
        self.vbo.release()