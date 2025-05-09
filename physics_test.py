import basilisk as bsk
import random
import glm

engine = bsk.Engine()
scene  = bsk.Scene(engine)
# scene.physics_engine.accelerations = []

# materials and meshes
red = bsk.Material(color=(255, 0, 0))
blue = bsk.Material(color=(0, 0, 255))
materials = [bsk.Material(color=(255 * (i & 4), 255 * (i & 2), 255 * (i & 1))) for i in range(1, 8)]

cube_mesh = engine.cube
cylinder_mesh = bsk.Mesh('tests/cylinder.obj')
sphere_mesh = bsk.Mesh('tests/sphere.obj', generate_bvh=False)
# monkey_mesh = bsk.Mesh('tests/monkey.obj')
# bunny_mesh = bsk.Mesh('tests/bunny.obj')
meshes = [cube_mesh]

scene.add(bsk.Node(
    mesh = sphere_mesh,
    scale = (1, 10, 1)
))

# creating nodes
platform = scene.add(bsk.Node(
    position=(0, -5, 0),
    scale=(10, 1, 10),
    # rotation=(-0.3, 0, 0),
    material=blue,
    collision=True,
))

radius = 4

objects = [scene.add(bsk.Node(
    position   = [random.uniform(-radius, radius), 0, random.uniform(-radius, radius)], 
    scale      = [random.uniform(0.5, 3) for _ in range(3)],
    rotation   = [random.uniform(0.1, 0.2) for _ in range(3)],
    rotational_velocity = (3.14159, 0, 0),
    mesh       = cube_mesh, 
    material   = red,
    static     = False,
    physics    = True,
    collision  = True
)) for _ in range(10)]

while engine.running:

    scene.update()

    for object in objects:
        if object.y < -50: 
            object.rotational_velocity = (0, 0, 0)
            object.position = (0, 5, 0)
            object.velocity = (0, 0, 0)

    engine.update()