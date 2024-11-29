import moderngl as mgl
import numpy as np
import glm
from math import cos, sin, atan2
from ..render.image import Image


class DrawHandler():
    engine: ...
    """Back reference to the parent engine"""
    scene: ...
    """Back reference to the parent scene"""
    ctx: mgl.Context
    """Back reference to the parent context"""
    program: mgl.Program
    """2D draw program"""
    draw_data: list[float]
    """Temporary buffer for user draw calls"""
    vbo: mgl.Buffer
    """Buffer for all 2D draws"""
    vao: mgl.VertexArray
    """VAO for rendering all 2D draw calls"""
    
    def __init__(self, scene) -> None:
        # Back references
        self.scene  = scene
        self.engine = scene.engine
        self.ctx    = scene.engine.ctx

        # Get the program
        self.program = self.scene.shader_handler.programs['draw']

        # Initialize draw data as blank
        self.draw_data = []
        self.vbo = None
        self.vao = None

    def render(self) -> None:
        """
        Renders all draw calls from the user since the last frame
        """
        
        if not self.draw_data: return

        # Reverse the draw order, and convert to C-like array
        self.draw_data.reverse()
        data = np.array(self.draw_data, dtype='f4')
        ratio = np.array([2 / self.engine.win_size[0], 2 / self.engine.win_size[1]])
        data[:,:2] = data[:,:2] * ratio - 1

        # Create buffer and VAO
        self.vbo = self.ctx.buffer(data)
        self.vao = self.ctx.vertex_array(self.program, [(self.vbo, '2f 4f 1i', *['in_position', 'in_color', 'in_uses_image'])], skip_errors=True)

        # Render the VAO
        self.vao.render()

        # Clera the draw data
        self.vbo.release()
        self.vao.release()
        self.vbo = None
        self.vao = None
        self.draw_data.clear()

    def draw_rect(self, color: tuple, rect: tuple) -> None:
        """
        Draws a rect to the screen
        """

        color = validate_color(color)
        rect  = validate_rect(rect)

        p1 = (rect[0]          , rect[1]          )
        p2 = (rect[0]          , rect[1] + rect[3])
        p3 = (rect[0] + rect[2], rect[1]          )
        p4 = (rect[0] + rect[2], rect[1] + rect[3])

        v1 = (*p1, *color, 0)
        v2 = (*p2, *color, 0)
        v3 = (*p3, *color, 0)
        v4 = (*p4, *color, 0)

        self.draw_data.extend([
            v1, v3, v2,
            v2, v3, v4
        ])

    def draw_circle(self, color: tuple, center: tuple, radius: int, resolution: int=20, outer_color: tuple=None) -> None:
        """
        Draws a rect between centered on x, y with width and height
            Args:
                color: tuple(r, g, b) | tuple(r, g, b, a)
                    The color value of the circle, with int components in range [0, 255]
                center: tuple (x: float, y: float)
                    Center of the circle, given in pixels
                radius: float
                    Radius of the circle, given in pixels
                resolution: float
                    The number of triangles used to approximate the circle
        """

        if not outer_color: outer_color = color
        color  = validate_color(color)
        outer_color  = validate_color(outer_color)
        p1 = validate_point(center)

        v1 = (*p1, *color, 0)
        theta = 0
        delta_theta = (2 * 3.1415) / resolution

        for triangle in range(resolution):
            v2 = (center[0] + radius * cos(theta), center[1] + radius * sin(theta), *outer_color, 0)
            theta += delta_theta
            v3 = (center[0] + radius * cos(theta), center[1] + radius * sin(theta), *outer_color, 0)
            self.draw_data.extend([v1, v2, v3])

    def draw_line(self, color: tuple, p1: tuple, p2: tuple, thickness: int=1):
        """
        Draws a line between two points
            Args:
                color: tuple=(r, g, b) | tuple=(r, g, b, a)
                    Color of the line
                p1: tuple=((x1, y1), (x2, y2))
                    Starting point of the line. Given in pixels
                p1: tuple=((x1, y1), (x2, y2))
                    Starting point of the line. Given in pixels
                thickness: int
                    Size of the line on either side. pixels
        """
        
        color = validate_color(color)

        p1 = glm.vec2(validate_point(p1))
        p2 = glm.vec2(validate_point(p2))

        thickness /= 2

        unit = glm.normalize(p1 - p2) * thickness
        theta = atan2(*unit)
        perp_vector = glm.vec2(cos(-theta), sin(-theta)) * thickness

        v1 = (*(p1 - perp_vector), *color, 0)
        v2 = (*(p1 + perp_vector), *color, 0)
        v3 = (*(p2 - perp_vector), *color, 0)
        v4 = (*(p2 + perp_vector), *color, 0)
        
        self.draw_data.extend([v1, v3, v4, v2, v1, v4])

    def blit(self, image: Image, rect: tuple):
        rect  = validate_rect(rect)

        p1 = (rect[0]          , rect[1]          )
        p2 = (rect[0]          , rect[1] + rect[3])
        p3 = (rect[0] + rect[2], rect[1]          )
        p4 = (rect[0] + rect[2], rect[1] + rect[3])

        v1 = (*p1, *image.index, 0, 0, 1)
        v2 = (*p2, *image.index, 0, 1, 1)
        v3 = (*p3, *image.index, 1, 0, 1)
        v4 = (*p4, *image.index, 1, 1, 1)

        self.draw_data.extend([
            v1, v3, v2,
            v2, v3, v4
        ])

    def __del__(self) -> None:
        """
        Releases any allocated data
        """
        
        if self.vbo: self.vbo.release()
        if self.vao: self.vao.release()


def validate_color(color):
    if not (isinstance(color, tuple) or isinstance(color, list) or isinstance(color, np.ndarray)):
        raise TypeError(f'Invalid color type: {type(color)}. Expected a tuple, list, or numpy array')
    if len(color) == 4:
        color = (color[0] / 255.0, color[1] / 255.0, color[2] / 255.0, color[3] / 255.0)
    elif len(color) == 3:
        color = (color[0] / 255.0, color[1] / 255.0, color[2] / 255.0, 1.0)
    else:
        raise TypeError(f'Invalid number of color values. Expected 3 or 4 values, got {len(color)}')
    return color

def validate_rect(rect):
    if not (isinstance(rect, tuple) or isinstance(rect, list) or isinstance(rect, np.ndarray)):
        raise TypeError(f'Invalid rect type: {type(rect)}. Expected a tuple, list, or numpy array')
    if len(rect) != 4:
        raise TypeError(f'Invalid number of rect values. Expected 4 values, got {len(rect)}')
    return list(rect)

def validate_point(point):
    if not (isinstance(point, tuple) or isinstance(point, list) or isinstance(point, np.ndarray)):
        raise TypeError(f'Invalid rect type: {type(point)}. Expected a tuple, list, or numpy array')
    if len(point) != 2:
        raise TypeError(f'Invalid number of rect values. Expected 2 values, got {len(point)}')
    return list(point)

def validate_image(image):
    if not (isinstance(image, Image)):
        raise TypeError(f'Invalid imgae type: {type(image)}. Expected a bask.Image.')
    return image