import basilisk as bsk

engine = bsk.Engine()
scene = bsk.Scene(engine)

sound = bsk.Sound('tests/john1.wav')
song = bsk.Sound('tests/selva_song.mp3')

song.play()

while engine.running:

    if engine.keys[bsk.pg.K_1]:
        sound.play(0)

    if engine.keys[bsk.pg.K_2]:
        song.play(1)
    if engine.keys[bsk.pg.K_3]:
        song.stop(1)

    if engine.keys[bsk.pg.K_UP]:
        song.volume += 5
    if engine.keys[bsk.pg.K_DOWN]:
        song.volume -= 5

    scene.update()
    engine.update()