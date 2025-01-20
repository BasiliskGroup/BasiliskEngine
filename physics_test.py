import basilisk as bsk
import random

engine = bsk.Engine()
scene  = bsk.Scene()
engine.scene = scene

# materials and meshes
red = bsk.Material(color=(255, 0, 0))
blue = bsk.Material(color=(0, 0, 255))
materials = [bsk.Material(color=(255 * (i & 4), 255 * (i & 2), 255 * (i & 1))) for i in range(1, 8)]

cube_mesh = engine.cube
# cylinder_mesh = bsk.Mesh('tests/cylinder.obj')
sphere_mesh = bsk.Mesh('tests/sphere.obj')
# monkey_mesh = bsk.Mesh('tests/monkey.obj')
# bunny_mesh = bsk.Mesh('tests/bunny.obj')
meshes = [cube_mesh]

# creating nodes
platform = scene.add_node(
    position=(0, -5, 0),
    scale=(10, 1, 10),
    # rotation=(-0.3, 0, 0),
    material=blue,
    collisions=True,
)

radius = 4

objects = [scene.add_node(
    position   = [random.uniform(-radius, radius), 0, random.uniform(-radius, radius)], 
    scale      = [random.uniform(2, 4) for _ in range(3)],
    rotation   = [random.uniform(1, 3) for _ in range(3)],
    mesh       = random.choice(meshes), 
    material   = red,
    collisions = True,
    physics    = True,
    static     = False,
    mass       = 1
) for _ in range(1)]

while engine.running:
    
    for object in objects:
        if object.y < -50: 
            object.rotational_velocity = (0, 0, 0)
            object.position = (0, 5, 0)
            object.velocity = (0, 0, 0)

    engine.update()