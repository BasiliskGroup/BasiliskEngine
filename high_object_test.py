import basilisk as bsk

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

n = 10
d = 5

for x in range(-n, n):
    for y in range(-n, n):
        for z in range(-n, n):
            scene.add_node(position=(x * d, y * d, z * d))

while engine.running:
    engine.update()