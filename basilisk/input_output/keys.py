import pygame as pg


class Keys:
    current_keys: list[bool]
    """The keys pressed during the current frame""" 
    previous_keys: list[bool]
    """The keys pressed in the last frame"""

    def __init__(self, engine: ...) -> None:
        """
        Handler for all keyboard inputs. Stores data from current and previous frames
        """
        
        # Reference to the parent engine
        self.engine = engine

        # Fill in default values for current and previous keys to aviod startup errors
        self.current_keys = pg.key.get_pressed()
        self.previous_keys = self.current_keys
        self.set_engine_attribiutes()


    def update(self) -> None:
        """
        Gets all keyboard inputs and propogates last frame inputs
        """

        # Get keyboard input
        self.previous_keys = self.current_keys
        # Propogate input to the last frame
        self.current_keys = pg.key.get_pressed()

        # Expose the attributes on the engine level
        self.set_engine_attribiutes()

    def set_engine_attribiutes(self) -> None:
        """
        Updates engine attributes with this instance's attributes for ease of use
        """

        setattr(self.engine, "keys", self.current_keys)
        setattr(self.engine, "previous_keys", self.previous_keys)
        setattr(self.engine, "prev_keys", self.previous_keys)