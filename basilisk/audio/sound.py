import os
import openal

class Sound:
    def __init__(self, path: str | os.PathLike):
        """
        Sound object that can be played
        """

        if not (isinstance(path, str) or isinstance(path, os.PathLike)):
            raise ValueError(f'bsk.Sound: Invalid source path type {type(path)}. Expected string or os.PathLike')

        self.source = openal.oalOpen(path)

    def play(self, volume: float=1.0):
        """
        Play the sound at the given volume level. Full volume if none given
        """

        self.source.set_gain(volume)
        self.source.play()

    def stop(self):
        """
        Stops the sound
        """

        self.source.stop()

    @property
    def isplaying(self):
        return self.source.get_state() == openal.AL_PLAYING