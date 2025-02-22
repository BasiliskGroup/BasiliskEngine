import pygame as pg


class Mouse():
    def __init__(self, grab=True):
        self._position = list(pg.mouse.get_pos())
        self._relative = [0, 0]
        self.buttons = pg.mouse.get_pressed()
        self.previous_buttons = pg.mouse.get_pressed()
        self.grab = grab
        self.visible = not self.grab

    def update(self, events):
        """
        Updates all mouse state variables.
        Checks for mouse-related events.
        """
        
        self._position = list(pg.mouse.get_pos())
        self._relative = list(pg.mouse.get_rel())
        self.previous_buttons = self.buttons
        self.buttons = pg.mouse.get_pressed()

        for event in events:
            if event.type == pg.KEYUP:
                if event.key == pg.K_ESCAPE and self.grab:
                    # Unlock mouse
                    self.grab = False
                    self.visible = True
            if event.type == pg.MOUSEBUTTONUP and not self.grab:
                # Lock mouse
                self.grab = True
                self.visible = False

    @property
    def position(self): return self._position
    @property
    def x(self): return self._position[0]
    @property
    def y(self): return self._position[1]
    @property
    def relative(self): return self._relative
    @property
    def relative_x(self): return self._relative[0]
    @property
    def relative_y(self): return self._relative[1]
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
    @property
    def visible(self): return self._visable

    @position.setter
    def position(self, value: tuple[int]) -> tuple[int]:
        self._position = value
        pg.mouse.set_pos(self._position)
        return self._position
    @x.setter
    def x(self, value: int) -> int:
        self._position[0] = value
        pg.mouse.set_pos(self._position)
        return self._position
    @y.setter
    def y(self, value: int) -> int:
        self._position[1] = value
        pg.mouse.set_pos(self._position)
        return self._position
    @grab.setter
    def grab(self, value) -> bool:
        self._grab = value
        pg.event.set_grab(self._grab)
        return self._grab
    @visible.setter
    def visible(self, value) -> bool:
        self._visible = value
        pg.mouse.set_visible(self._visible)
        return self._visible