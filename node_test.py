import basilisk as bsk

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

scene.camera = bsk.StaticCamera()

node = bsk.Node()
scene.add(node)

cube   = bsk.Mesh('tests/cube.obj')
monkey = bsk.Mesh('tests/monkey.obj')

while engine.running:

    dt = engine.delta_time

    if engine.keys[bsk.pg.K_1] and not node:
        node = scene.add(bsk.Node())
    if engine.keys[bsk.pg.K_2]:
        node = scene.remove(node)
    if engine.keys[bsk.pg.K_3] and node:
        node.x += 5 * dt
    if engine.keys[bsk.pg.K_4] and node:
        node.x -= 5 * dt
    if engine.keys[bsk.pg.K_5] and node:
        node.position.x += 5 * dt
    if engine.keys[bsk.pg.K_6] and node:
        node.scale.x += 5 * dt
    if engine.keys[bsk.pg.K_7] and node:
        node.rotation.x += 5 * dt
    if engine.keys[bsk.pg.K_8] and node:
        node.mesh = cube
    if engine.keys[bsk.pg.K_9] and node:
        node.mesh = monkey


    engine.update()