from ..engine import Engine
from ..render.image import Image

def rect(engine: Engine, color: tuple, rect: tuple) -> None:
    """
    Draws a rectagle to the screen
    Args:
        engine: bsk.Engine
            The destination engine for the rectangle
        color: tuple(r, g, b) | tuple(r, g, b, a)
            The color value of the rectangle, with int components in range [0, 255]
        rect: tuple(x, y, w, h)
            The screen position and size of the rectangle given in pixels
    """
    
    # Get the draw handler from the engine
    draw_handler = engine.draw_handler
    if not draw_handler: return

    # Draw the rect
    draw_handler.draw_rect(color, rect)


def circle(engine: Engine, color: tuple, center: tuple, radius: int, resolution: int=20, outer_color: tuple=None) -> None:
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
    
    # Get the draw handler from the engine
    draw_handler = engine.draw_handler
    if not draw_handler: return

    # Draw the circle
    draw_handler.draw_circle(color, center, radius, resolution, outer_color)

def line(engine: Engine, color: tuple, p1: tuple, p2: tuple, thickness: int=1) -> None:
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
    
    # Get the draw handler from the engine
    draw_handler = engine.draw_handler
    if not draw_handler: return

    # Draw the line
    draw_handler.draw_line(color, p1, p2, thickness)

def blit(engine: Engine, image: Image, rect: tuple, alpha: float=1.0):
    """
    Blits a basilisk image to the engine screen.
    Args:
        image: bsk.Image
            The image to display on the screen
        rect: tuple(x, y, w, h)
            The screen position and size of the image given in pixels
    """

    # Get the draw handler from the engine
    draw_handler = engine.draw_handler
    if not draw_handler: return

    engine.material_handler.image_handler.add(image)

    # Blit the image
    draw_handler.blit(image, rect, alpha)

def text(engine: Engine, text: str, position: tuple, scale: float=1.0):
    """
    Renders text do the screen
    USE SPARINGLY, INEFFICIENT IMPLAMENTATION
    """
    
    font_renderer = engine.draw_handler.font_renderer

    # Render the text if it has not been cached
    if text not in font_renderer.text_renders:
        surf = font_renderer.render(text).convert_alpha()
        text_image = Image(surf)
        font_renderer.text_renders[text] = (text_image, surf.get_rect())
    
    # Blit the text image
    img, rect = font_renderer.text_renders[text]
    blit(engine, img, (position[0] - rect[2] * scale / 2, position[1] - rect[3] * scale / 2, rect[2] * scale, rect[3] * scale))