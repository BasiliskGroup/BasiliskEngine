import basilisk as bsk

engine = bsk.Engine(title=None)
scene = bsk.Scene(engine)
engine.config.bloom_enabled = False

scene.add(bsk.Node())

while engine.running:
    print('-----------')
    scene.update()
    engine.update(render=False)