import basilisk as bsk
from math import cos, sin, pi
import glm
import time
import random

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

cube_mesh = bsk.cube
sphere_mesh = bsk.Mesh('tests/sphere.obj')

mud = bsk.Image("tests/mud.png")
mud_normal = bsk.Image("tests/mud_normal.png")
mud_mtl = bsk.Material(texture=mud, normal=mud_normal)
materials = [bsk.Material(color=(255 * (i & 4), 255 * (i & 2), 255 * (i & 1))) for i in range(1, 8)]

for i in range(3):
    node = scene.add_node(
        position=[random.uniform(-10, 10), random.uniform(-3, -10), random.uniform(-10, 10)], 
        scale=[random.uniform(0.5, 2) for _ in range(3)],
        rotation=[random.uniform(-10, 10), random.uniform(-3, -10), random.uniform(-10, 10)], 
        mesh=cube_mesh, 
        material=mud_mtl,
        # velocity=(0, 0.1, 0),
        # rotational_velocity=(0, 1, 0),
        # physics=True,
        # mass=1,
        collisions=True
    )
    
    vertices = node.collider.obb_points
    
    for vertex in vertices:
        scene.add_node(
            position = vertex,
            scale = (0.1, 0.1, 0.1),
            material = materials[6],
            mesh=cube_mesh
        )
    
    # edges = [
    #     (vertices[0], vertices[1]),
    #     (vertices[0], vertices[2]),
    #     (vertices[0], vertices[4]),
    #     (vertices[1], vertices[3]),
    #     (vertices[1], vertices[5]),
    #     (vertices[2], vertices[3]),
    #     (vertices[2], vertices[6]),
    #     (vertices[3], vertices[7]),
    #     (vertices[4], vertices[5]),
    #     (vertices[4], vertices[6]),
    #     (vertices[5], vertices[7]),
    #     (vertices[6], vertices[7]),
    # ]
    
    # for edge in edges:
    #     center = (edge[1] + edge[0]) / 2
    #     dimensions = glm.abs(edge[1] - edge[0]) + glm.vec3(0.01)
    #     dimensions /= 2
    #     scene.add_node(
    #         position = center,
    #         scale = dimensions,
    #         material=materials[3],
    #         mesh=cube_mesh
    #     )

while engine.running:
    engine.update()