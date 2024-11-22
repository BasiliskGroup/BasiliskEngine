import cv2
import pygame as pg
import numpy as np

class VideoPlayer:
    def __init__(self) -> None:
        """
        Creates a video player that can be used to play videos in pygame with synced audio
        """

        # Video state vairables
        self.is_playing = False
        self.paused = False
        self.frame = None
        self.cap = None

        # Initalize audio
        pg.mixer.pre_init(44100, -16, 2, 512)
        pg.mixer.init()
        pg.mixer.set_num_channels(64)
        pg.mixer.music.set_volume(75)

    def play(self, video: str, audio: str=None) -> None:
        """
        Plays the given video and audio in sync.
        Cannot read MP4 audio, so must provide an audio file if you want sound.
        """

        # Release any in use captures
        if self.is_playing: self.cap.release()
        # Set the video capture input
        self.cap = cv2.VideoCapture(video)

        # Clear any leftover frames
        self.frame = None

        # Frame time variables 
        self.seconds_per_frame = 1.0 / self.cap.get(cv2.CAP_PROP_FPS)
        self.current_frame = 0

        # Play video audio
        if audio:
            pg.mixer.music.load(audio)
            pg.mixer.music.play()

        # Set flags
        self.is_playing = True
        self.paused = False

    def stop(self) -> None:
        """
        Stops the currently playing video
        """
        
        # Relase the video input
        self.cap.release()
        # Stop video audio
        pg.mixer.music.stop()
        # Set is playing flag
        self.is_playing = False

    def set_volume(self, volume: int) -> None:
        """
        Wrapper for the pg.mixer.music.set_volume function
        """
        
        pg.mixer.music.set_volume(volume)

    def pause(self) -> None:
        """
        Pauses the video and audio
        """
        
        pg.mixer.music.pause()
        self.paused = True

    def unpause(self) -> None:
        """
        Resumes the video and audio from a pause
        """
        
        pg.mixer.music.unpause()
        self.paused = False

    def get_frame(self, dt) -> pg.Surface:
        """
        Gets the currently playing video's frame. 
        Returns a pygame surface of the frame.
        Args:
            dt: float
                The amount of time (in seconds) that has passed since last call to this function (i.e. pg.Clock.tick() / 1000)
        """
        
        # Read the frame
        if not self.is_playing:
            return None
        
        # Update the current frame's time
        self.current_frame += dt * (not self.paused)
        if self.current_frame <= self.seconds_per_frame:
            return self.frame

        # Get the number of video frames that have passed since the last get_frame call
        frames_to_read = int(self.current_frame // self.seconds_per_frame)
        # Read frames until we are at the current frame
        for f in range(frames_to_read):
            ret, frame = self.cap.read()
            if not ret: self.stop(); return None

        # Reset the current frame time, leave leftover time
        self.current_frame %= self.seconds_per_frame


        # Make the surface from the frame
        frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        frame = np.rot90(frame)
        frame = pg.surfarray.make_surface(frame)

        # Flip orientation
        self.frame = pg.transform.flip(frame, True, False)
        
        # Blit to screen
        return self.frame