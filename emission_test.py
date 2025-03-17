import basilisk as bsk

engine = bsk.Engine(title=None)
scene = bsk.Scene(engine)

emissive_mtl = bsk.Material(emissive_color=(2 * 255, 1.5 * 255, 50))

sphere_mesh = bsk.Mesh('tests/sphere.obj')

node = bsk.Node(material=emissive_mtl)
scene.add(node)


while engine.running:

    if engine.keys[bsk.pg.K_q]: engine.config.bloom_enabled = True
    if engine.keys[bsk.pg.K_e]: engine.config.bloom_enabled = False

    if engine.keys[bsk.pg.K_1]: engine.config.bloom_quality = 1
    if engine.keys[bsk.pg.K_2]: engine.config.bloom_quality = 2
    if engine.keys[bsk.pg.K_3]: engine.config.bloom_quality = 3
    if engine.keys[bsk.pg.K_4]: engine.config.bloom_quality = 4
    if engine.keys[bsk.pg.K_5]: engine.config.bloom_quality = 5
    if engine.keys[bsk.pg.K_6]: engine.config.bloom_quality = 6
    if engine.keys[bsk.pg.K_7]: engine.config.bloom_quality = 7
    if engine.keys[bsk.pg.K_8]: engine.config.bloom_quality = 8

    scene.update()
    engine.update()