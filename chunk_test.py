import basilisk as bsk

engine = bsk.Engine()
scene = bsk.Scene(engine)

cube = bsk.Node(position=(5, 0, 0))

cube2 = bsk.Node(position=(5, 4, 0))

scene.add(cube, cube2)

while engine.running:

    speed = 10
    cube.x += (engine.keys[bsk.pg.K_RIGHT] - engine.keys[bsk.pg.K_LEFT]) * speed * engine.delta_time
    cube.y += (engine.keys[bsk.pg.K_UP] - engine.keys[bsk.pg.K_DOWN]) * speed * engine.delta_time

    if engine.keys[bsk.pg.K_i] and not engine.previous_keys[bsk.pg.K_i]:
        print(cube)
        print(cube.chunk)

    scene.update()
    engine.update()