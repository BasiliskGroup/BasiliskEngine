from math import ceil


class ViewportDimensions:
    def __init__(self, top: float=0, bottom: float=0, left: float=0, right: float=0, callback=None) -> None:
        self.callback = callback
        """Function that is called if any of the viewport dimensions are changed."""

        self.top:    float = top
        """Distance from the top of the screen. Value from (0, 1). Contains the toolbar viewport."""
        self.bottom: float = bottom
        """Distance from the bottom of the screen. Value from (0, 1). Contains the project file viewport."""
        self.left:   float = left
        """Distance from the left of the screen. Value from (0, 1). Contains the heirarchy viewport."""
        self.right:  float = right
        """Distance from the right of the screen. Value from (0, 1). Contains the inspector viewport."""

    @property
    def top(self):    return self._top
    @property
    def bottom(self): return self._bottom
    @property
    def left(self):   return self._left
    @property
    def right(self):  return self._right
    
    @property
    def width(self):
        """Width of the central viewport from the inner edge of the left window to the inner edge of the right window."""
        return min(max(1 - self._right - self._left, 0.0), 1.0)
    @property
    def height(self):
        """Height of the central viewport from the inner edge of the top window to the inner edge of the bottom window."""
        return min(max(1 - self._top - self._bottom, 0.0), 1.0)

    @top.setter
    def top(self, value: float):
        self._top = value
        if self.callback: self.callback()
    @bottom.setter
    def bottom(self, value: float):
        self._bottom = value
        if self.callback: self.callback()
    @left.setter
    def left(self, value: float):
        self._left = value
        if self.callback: self.callback()
    @right.setter
    def right(self, value: float):
        self._right = value
        if self.callback: self.callback()

    def get_toolbar_pixels(self, win_size):
        """Returns the size of the toolbar in pixels given a win_size"""
        return (win_size[0], ceil(win_size[1] * self.top))
    
    def get_hierarchy_pixels(self, win_size):
        """Returns the size of the hierarchy in pixels given a win_size"""
        return (ceil(win_size[0] * self.left),  ceil(win_size[1] * (1 - (self.top + self.bottom))))
    
    def get_inspector_pixels(self, win_size):
        """Returns the size of the inspector in pixels given a win_size"""
        return (ceil(win_size[0] * self.right), ceil(win_size[1] * (1 - (self.top + self.bottom))))
    
    def get_project_files_pixels(self, win_size):
        """Returns the size of the project files view in pixels given a win_size"""
        return (win_size[0], ceil(win_size[1] * self.bottom))
    
    def get_viewport_pixels(self, win_size):
        """Returns the size of the viewport in pixels given a win_size"""
        return (ceil(win_size[0] * self.width), ceil(win_size[1] * self.height))