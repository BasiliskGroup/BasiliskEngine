import basilisk as bsk
import random

engine = bsk.Engine(title=None)
scene = bsk.Scene(engine)

monkey = bsk.Mesh('tests/monkey.obj')
cylinder = bsk.Mesh('tests/cylinder.obj')
sphere = bsk.Mesh('tests/sphere.obj')
meshes = [None, monkey, cylinder, sphere]

red = bsk.Material(color=(255, 0, 0))
floor_img = bsk.Image('tests/floor_albedo.png')
floor_norm = bsk.Image('tests/floor_normal.png')
floor = bsk.Material(texture=floor_img, normal=floor_norm)

materials = [None, red, floor]

while engine.running:
    scene.update()
    
    if engine.keys[bsk.pg.K_1]:
        scene.particle.add()

    if engine.keys[bsk.pg.K_2]:
        scene.particle.add(velocity=[random.randrange(-5, 5) for i in range(3)])

    if engine.keys[bsk.pg.K_3]:
        scene.particle.add(material=red, velocity=[random.randrange(-5, 5) for i in range(3)])

    if engine.keys[bsk.pg.K_4]:
        scene.particle.add(material=floor, velocity=(0, 0, 0), acceleration=(0, 0, 0), scale=3)
    
    if engine.keys[bsk.pg.K_5]:
        scene.particle.add(velocity=(0, 0, 0), acceleration=[random.randrange(-5, 5) for i in range(3)])

    if engine.keys[bsk.pg.K_6]:
        scene.particle.add(mesh=monkey, velocity=[random.randrange(-5, 5) for i in range(3)])
    
    if engine.keys[bsk.pg.K_7]:
        scene.particle.add(mesh=meshes[random.randrange(len(meshes))], velocity=[random.randrange(-5, 5) for i in range(3)])
    
    if engine.keys[bsk.pg.K_8]:
        scene.particle.add(mesh=meshes[random.randrange(len(meshes))], material=materials[random.randrange(len(materials))], velocity=[random.randrange(-5, 5) for i in range(3)])
    
    engine.update()