import pygame as pg
import random

music = {
    'song' : 'audio/music/song.mp3',
}

sounds = {
    'door' : 'audio/sounds/door1.mp3',
    'hit1' : 'audio/sounds/hit1.mp3',
    'hit2' : 'audio/sounds/hit2.mp3',
    'hit3' : 'audio/sounds/hit3.mp3',
    'hit4' : 'audio/sounds/hit4.mp3',
    'hit5' : 'audio/sounds/hit5.mp3',
    'slice1' : 'audio/sounds/slice1.mp3',
    'slice2' : 'audio/sounds/slice2.mp3',
    'slice3' : 'audio/sounds/slice3.mp3',
    'step1' : 'audio/sounds/step1.mp3',
    'step2' : 'audio/sounds/step2.mp3',
    'step3' : 'audio/sounds/step3.mp3',
    'step4' : 'audio/sounds/step4.mp3',
    'john1' : 'audio/sounds/john1.mp3',
    'john2' : 'audio/sounds/john2.mp3'
}

class AudioHandler:
    def __init__(self):
        pg.mixer.pre_init(44100, -16, 2, 512)
        pg.mixer.init()
        pg.mixer.set_num_channels(64)
        pg.mixer.music.set_volume(100/100)
        MUSIC_END = pg.USEREVENT+1
        pg.mixer.music.set_endevent(MUSIC_END)
        
        self.sounds = {sound : pg.mixer.Sound(sounds[sound]) for sound in sounds}
        self.playlists = {}
        self.sound_groups = {}

        self.make_playlist('music', ['song'])

        self.add_sound_group('door', ('door'))
        self.add_sound_group('hit', ('hit1', 'hit2', 'hit3', 'hit4', 'hit5'))
        self.add_sound_group('slice', ('slice1', 'slice2', 'slice3'))
        self.add_sound_group('step', ('step1', 'step2', 'step3', 'step4'))
        self.add_sound_group('john', ('john1', 'john2'))

        self.current_track = None
        self.current_playlist = None
        self.playlist_index = 0

    def update_volume(self):
        pg.mixer.music.set_volume(100/100)

    def make_playlist(self, name, songs):
        self.playlists[name] = [music[song] for song in songs]

    def add_sound_group(self, name: str, sounds: tuple):
        self.sound_groups[name] = sounds

    def play_playlist(self, playlist):
        self.playlist_index = 0
        if self.current_track or self.current_playlist: 
            pg.mixer.music.fadeout(1000)
            self.playlist_index = -1
        self.current_playlist = playlist
        pg.mixer.music.load(self.playlists[playlist][self.playlist_index])
        pg.mixer.music.play(fade_ms=1000)

    def update_playlist(self):
        if not self.current_playlist: return
        self.playlist_index += 1
        if self.playlist_index >= len(self.playlists[self.current_playlist]): self.playlist_index = 0
        pg.mixer.music.load(self.playlists[self.current_playlist][self.playlist_index])
        pg.mixer.music.play(fade_ms=1000)

    def play_track(self, song):
        if self.current_track or self.current_playlist: pg.mixer.music.fadeout(1000)
        self.current_playlist = None
        pg.mixer.music.load(music[song])
        pg.mixer.music.play(-1, fade_ms=1000)
        self.current_track = song

    def play_sound(self, sound):
        snd = self.sounds[sound]
        snd.set_volume(100/100)
        snd.play()

    def play_sound_group(self, group_name):
        sound = random.choice(self.sound_groups[group_name])
        self.play_sound(sound)

    def fast_forward(self):
        pg.mixer.music.pause()
        pg.mixer.music.set_pos(125)
        pg.mixer.music.unpause()