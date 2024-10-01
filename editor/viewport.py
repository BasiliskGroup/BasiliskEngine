import pygame as pg


class Viewport:
    def __init__(self, editor) -> None:
        # Reference to enviornment
        self.editor       = editor
        self.engine       = self.editor.engine
        self.viewport_dim = editor.viewport_dim

        self.set_surf()  # Set the surface for drawing

    def render(self):
        """Clears the viewport and renders all elements"""
        self.surf.fill((0, 0, 0, 0))  # Must be transparent to see the camera view behind it.

    def get_input(self) -> None:
        ...

    def scroll(self, value) -> None:
        ...

    def set_surf(self) -> None:
        """Sets the viewport surface for drawing onto."""
        self.dim = self.viewport_dim.get_viewport_pixels(self.engine.win_size)
        self.surf = pg.Surface(self.dim).convert_alpha()