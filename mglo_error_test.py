import basilisk as bsk

engine = bsk.Engine()
scene = bsk.Scene(engine)

cube   = bsk.Mesh('tests/cube.obj')
monkey = bsk.Mesh('tests/monkey.obj')

print(f'cube: {cube}')
print(f'monkey: {monkey}')

node = bsk.Node(physics=True, collision=True)
floor = bsk.Node(position=(0, -5, 0), scale=(10, .5, 10), physics=False, collision=True)
scene.add(node)

while engine.running:
    scene.update()

    if engine.keys[bsk.pg.K_1]:
        node.mesh = cube
    if engine.keys[bsk.pg.K_2]:
        node.mesh = monkey

    node.position.x += (engine.keys[bsk.pg.K_RIGHT] - engine.keys[bsk.pg.K_LEFT]) * 5 * engine.delta_time
    node.scale.x += (engine.keys[bsk.pg.K_UP] - engine.keys[bsk.pg.K_DOWN]) * 5 * engine.delta_time

    engine.update()