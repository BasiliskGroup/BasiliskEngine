import pygame as pg


class Mouse():
    def __init__(self, grab=True):
        self.x, self.y = pg.mouse.get_pos()
        self.buttons = pg.mouse.get_pressed()
        self.previous_buttons = pg.mouse.get_pressed()
        self.grab = grab

    def update(self, events):
        """
        Updates all mouse state variables.
        Checks for mouse-related events.
        """
        
        self.x, self.y = pg.mouse.get_pos()
        self.previous_buttons = self.buttons
        self.buttons = pg.mouse.get_pressed()

        for event in events:
            if event.type == pg.KEYUP:
                if event.key == pg.K_ESCAPE and self.grab:
                    # Unlock mouse
                    pg.event.set_grab(False)
                    pg.mouse.set_visible(True)
            if event.type == pg.MOUSEBUTTONUP and self.grab:
                # Lock mouse
                pg.event.set_grab(True)
                pg.mouse.set_visible(False)

    def set_pos(self, x, y):
        """Set the mouse position"""
        pg.mouse.set_pos(x, y)

    @property
    def click(self): return self.buttons[0] and not self.previous_buttons[0]
    @property
    def left_click(self): return self.buttons[0] and not self.previous_buttons[0]
    @property
    def middle_click(self): return self.buttons[1] and not self.previous_buttons[1]
    @property
    def right_click(self): return self.buttons[2] and not self.previous_buttons[2]
    @property
    def left_down(self): return self.buttons[0]
    @property
    def middle_down(self): return self.buttons[1]
    @property
    def right_down(self): return self.buttons[2]

    @property
    def grab(self): return self._grab

    @grab.setter
    def grab(self, value):
        self._grab = value
        if self._grab:
            pg.event.set_grab(True)
            pg.mouse.set_visible(False)
        else:
            pg.event.set_grab(False)
            pg.mouse.set_visible(True)