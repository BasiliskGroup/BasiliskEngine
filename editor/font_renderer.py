import pygame

class FontRenderer():
    def __init__(self):
        self.fonts = {}
        self.load_font("Roboto", "scripts/editor/editor_assets/RobotoMono-Regular.ttf")

    def load_font(self, name, path):
        self.fonts[name] = []
        self.fonts[name].append(pygame.font.Font(path, 12))
        self.fonts[name].append(pygame.font.Font(path, 22))
        self.fonts[name].append(pygame.font.Font(path, 44))
        self.fonts[name].append(pygame.font.Font(path, 10))
    
    def render_text(self, win, pos, text, font="Roboto", size=1, color=(255, 255, 255), bold=False, underline=False, italic=False, center_width=False):
        '''
        Renders any font which has been loaded to the class instance.
        Args:
            win::pygame.surface
                This is the drawing location for the text
            pos::(int, int)
                The X and Y Coordinates for the text to be drawn
            text::str
                Text to be rendered
            font::str (="Calibri")
                Any loaded or standard font can be specified
            size::int (=1)
                Sets the size of the text. Value can be either 0, 1, or 2, which is small, medium, or large.
            color::(int, int, int) =(255, 255, 255)
                The RGB value of the text
            bold::bool (=False)
                Specifies if the text should be rendered in bolded font
            underline::bool (=False)
                Specifies if the text should be underlined in bolded font
            italic::bool (=False)
                Specifies if the text should be rendered in italicized font
            center_width::bool (=False)
                If True, the text will be horizontaly centered on the X position provided by pos. Otherwise, the leftmost side will be on the X coordinate
        '''
        self.fonts[font][size].set_bold(bold)
        self.fonts[font][size].set_underline(underline)
        self.fonts[font][size].set_italic(italic)

        text_img = self.fonts[font][size].render(text, True, color)

        if center_width:
            win.blit(text_img, (pos[0] - text_img.get_width() / 2, pos[1] - self.fonts[font][size].get_height() / 2))
        else:
            win.blit(text_img, (pos[0], pos[1] - self.fonts[font][size].get_height() // 2))