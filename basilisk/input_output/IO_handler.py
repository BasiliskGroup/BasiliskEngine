import pygame as pg
from .keys import Keys
from .mouse import Mouse


class IO:
    keys: Keys=None
    """Handler for all keyboard inputs""" 
    mouse: Mouse=None
    """Handler for all mouse inputs and mouse settings"""
    events: list=[]
    """List of all the events in the frame"""
    event_resize: bool=False
    """Bool for if a resize event has occured this frame"""

    def __init__(self, engine: ..., grab_mouse: bool=True, caption: bool|None="Basilisk") -> None:
        """
        Class to handle all inputs and outputs for the engine. 
        """
        
        # Reference to parent engine
        self.engine = engine

        # Caption attrib
        self.caption = caption
        self.update_caption()

        # Handlers for key and mouse input
        self.keys = Keys(engine)
        self.mouse = Mouse(grab=grab_mouse)

        # Expose the mouse on the engine level
        setattr(self.engine, "mouse", self.mouse)

        # Fill in default values for engine attributes
        self.set_engine_attribiutes()

        # Update the icon for the window
        pg.display.set_icon(pg.image.load(self.engine.root + '/bsk_assets/basilisk.png'))

    def update(self) -> None:
        """
        Update all inputs and check for events
        """

        # Get events
        self.events = pg.event.get()

        # Update the keys and the mouse
        self.keys.update()
        self.mouse.update(self.events)

        # Handle events and update attributes
        self.get_events()
        self.set_engine_attribiutes()
        self.update_caption()

    def get_events(self) -> None:
        """
        Loop through all pygame events and make updates as needed
        """
        
        # Clear global events
        self.event_resize = False


        for event in self.events:
            if event.type == pg.QUIT: # Quit the engine
                self.engine.quit()
                return
            if event.type == pg.VIDEORESIZE:
                # Updates the viewport
                self.event_resize = True
                self.engine.win_size = (event.w, event.h)
                self.engine.ctx.viewport = (0, 0, event.w, event.h)
                for fbo in self.engine.fbos: fbo.resize()

    def update_caption(self) -> None:
        """
        Updates the window caption with either the fps or the window name. Set to 'Basilisk Engine' by default
        """
        
        caption = self.caption if self.caption else f"FPS: {round(self.engine.clock.fps)}"
        pg.display.set_caption(caption)

    def set_engine_attribiutes(self) -> None:
        """
        Updates engine attributes with this instance's attributes for ease of use
        """

        setattr(self.engine, "events", self.events)
        setattr(self.engine, "event_resize", self.event_resize)