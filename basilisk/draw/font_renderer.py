import pygame as pg

class FontRenderer():
    def __init__(self, root):
        pg.font.init()
        self.font = pg.font.Font(root + '/bsk_assets/Roboto-Regular.ttf', 48)
        self.text_renders = {}

    def render(self, text, color=(255, 255, 255), bold=False, underline=False, italic=False):
        '''
        Renders any font which has been loaded to the class instance.
        Args:
            text::str
                Text to be rendered
            color::(int, int, int) =(255, 255, 255)
                The RGB value of the text
            bold::bool (=False)
                Specifies if the text should be rendered in bolded font
            underline::bool (=False)
                Specifies if the text should be underlined in bolded font
            italic::bool (=False)
                Specifies if the text should be rendered in italicized font
        '''
        self.font.set_bold(bold)
        self.font.set_underline(underline)
        self.font.set_italic(italic)

        return self.font.render(text, True, color, (0, 0, 0, 0))
