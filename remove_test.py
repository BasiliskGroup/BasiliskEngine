import basilisk as bsk

engine = bsk.Engine()
scene = bsk.Scene(engine)

nodes = []

for x in range(-1, 2):
    for y in range(-1, 2):
        for z in range(-1, 2):
            nodes.append(scene.add(bsk.Node(position=(x * 3, y * 3, z * 3))))

while engine.running:
    scene.update()
    if engine.keys[bsk.pg.K_r] and not engine.previous_keys[bsk.pg.K_r] and len(nodes):
        scene.remove(nodes[-1])
        nodes.pop(-1)
    engine.update()
