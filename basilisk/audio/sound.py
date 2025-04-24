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
        self.volume = 100

    def play(self, fade: float=0.0):
        """
        Play the sound at the given volume level. Fades in over `fade` seconds
        """

        self.source.play(fade_ms=int(fade * 1000))

    def stop(self, fade: float=0.0):
        """
        Stops the sound. Fades out over `fade` seconds
        """

        self.source.fadeout(int(fade * 1000))

    @property
    def volume(self) -> float: 
        return self._volume

    @volume.setter
    def volume(self, value: int) -> None:
        """
        Sets the volume of the music. Int from [0, 100]
        """

        self._volume = min(max(value, 0), 100)
        self.source.set_volume(self._volume / 100)