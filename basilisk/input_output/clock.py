import pygame as pg


class Clock:
    delta_time: float=0
    """The amount of time that passed between the last and current frame"""
    time: float=0
    """Total time that has passed since the start of the program"""

    def __init__(self, engine, max_fps=None) -> None:
        """
        Class to keep track of all time and delta time in the program
        """

        # Reference to the parent engine
        self.engine = engine

        # Create a pygame clock
        self.clock = pg.Clock()
        self.max_fps = max_fps

        # Default values for attributes
        self.set_engine_attribiutes()

    def update(self) -> None:
        """Ticks the clock"""
        
        # Tick the clock and get delta time in seconds
        if self.max_fps: self.delta_time = self.clock.tick(self.max_fps) / 1000
        else: self.delta_time = self.clock.tick() / 1000

        # Increment the total time by time since last frame
        self.time += self.delta_time

        self.set_engine_attribiutes()

    def set_engine_attribiutes(self) -> None:
        """
        Updates engine attributes with this instance's attributes for ease of use
        """
        
        setattr(self.engine, "delta_time", self.delta_time)
        setattr(self.engine, "dt", self.delta_time)
        setattr(self.engine, "time", self.time)
        setattr(self.engine, "fps", self.fps)


    @property
    def fps(self) -> float:
        return self.clock.get_fps()