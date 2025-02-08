import basilisk as bsk

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

sound = bsk.Sound('tests/john1.wav')

while engine.running:

    if engine.keys[bsk.pg.K_1]:
        sound.play()

    engine.update()
