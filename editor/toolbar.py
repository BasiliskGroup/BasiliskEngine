import pygame as pg


class Toolbar:
    def __init__(self, editor) -> None:
        # Reference to enviornment
        self.editor       = editor
        self.engine       = self.editor.engine
        self.viewport_dim = editor.viewport_dim

        # Display Settings
        self.padding = 3
        self.button_size = 80

        self.set_surf()  # Set the surface for drawing

    def render(self):
        """Clears the viewport and renders all elements"""
        self.surf.fill(self.editor.ui.primary)

        self.surf.blit(self.editor.ui.logo, (self.dim[1] * .1, self.dim[1] * .1))

        self.editor.font.render_text(self.surf, (self.dim[1] + self.padding + self.button_size * 0, self.dim[1] / 2), "File")
        self.editor.font.render_text(self.surf, (self.dim[1] + self.padding + self.button_size * 1, self.dim[1] / 2), "Edit")
        self.editor.font.render_text(self.surf, (self.dim[1] + self.padding + self.button_size * 2, self.dim[1] / 2), "View")

        pg.draw.rect(self.surf, self.editor.ui.outline, (0, 0, self.dim[0], self.dim[1]), 1)

    def get_input(self) -> None:
        ...

    def scroll(self, value) -> None:
        ...

    def set_surf(self) -> None:
        """Sets the viewport surface for drawing onto."""
        self.dim = self.viewport_dim.get_toolbar_pixels(self.engine.win_size)
        self.surf = pg.Surface(self.dim).convert_alpha()