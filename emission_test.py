import basilisk as bsk

engine = bsk.Engine(title=None)
scene = bsk.Scene(engine)

img = bsk.Image('tests/mud.png')
emissive_mtl = bsk.Material(texture=img, emissive_color=(1 * 255, .5 * 255, 50))

sphere_mesh = bsk.Mesh('tests/sphere.obj')

node = bsk.Node(material=emissive_mtl)
scene.add(node)


while engine.running:

    if engine.keys[bsk.pg.K_q]: engine.config.bloom_enabled = True
    if engine.keys[bsk.pg.K_e]: engine.config.bloom_enabled = False

    scene.update()
    engine.update()