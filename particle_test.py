import basilisk as bsk
import random

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

monkey = bsk.Mesh('tests/monkey.obj')
cylinder = bsk.Mesh('tests/cylinder.obj')
sphere = bsk.Mesh('tests/sphere.obj')
meshes = [None, monkey, cylinder, sphere]

while engine.running:
    engine.update()
    
    if engine.keys[bsk.pg.K_1]:
        scene.particle.add()

    if engine.keys[bsk.pg.K_2]:
        scene.particle.add(color=(225, 50, 50))

    if engine.keys[bsk.pg.K_3]:
        scene.particle.add(velocity=(random.randrange(-5, 5) for i in range(3)))
    
    if engine.keys[bsk.pg.K_4]:
        scene.particle.add(velocity=(0, 0, 0), acceleration=(random.randrange(-5, 5) for i in range(3)))

    if engine.keys[bsk.pg.K_5]:
        scene.particle.add(mesh=monkey, velocity=(random.randrange(-5, 5) for i in range(3)))
    
    if engine.keys[bsk.pg.K_6]:
        scene.particle.add(mesh=meshes[random.randrange(len(meshes))], velocity=(random.randrange(-5, 5) for i in range(3)))