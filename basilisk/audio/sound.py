import os
import pygame as pg

class Sound:
    def __init__(self, path: str | os.PathLike):
        """
        Sound object that can be played
        """

        if not (isinstance(path, str) or isinstance(path, os.PathLike)):
            raise ValueError(f'bsk.Sound: Invalid source path type {type(path)}. Expected string or os.PathLike')

        self.source = pg.mixer.Sound(path)

    def play(self, volume: float=1.0):
        """
        Play the sound at the given volume level. Full volume if none given
        """

        self.source.set_volume(volume)
        self.source.play()

    def stop(self):
        """
        Stops the sound
        """

        self.source.stop()