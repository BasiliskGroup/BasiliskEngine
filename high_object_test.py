import basilisk as bsk

engine = bsk.Engine()
scene = bsk.Scene(engine)

n = 10
d = 5

for x in range(-n, n):
    for y in range(-n, n):
        for z in range(-n, n):
            scene.add(bsk.Node(position=(x * d, y * d, z * d)))

while engine.running:
    scene.update()
    engine.update()