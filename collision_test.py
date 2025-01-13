import basilisk as bsk
import pygame as pg
import random

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

# meshes
cube_mesh = engine.cube
sphere_mesh = bsk.Mesh('tests/sphere.obj')

meshes = [sphere_mesh, cube_mesh]

# materials
mud = bsk.Image("tests/mud.png")
mud_normal = bsk.Image("tests/mud_normal.png")
mud_mtl = bsk.Material(texture=mud, normal=mud_normal)
materials = [bsk.Material(color=(255 * (i & 4), 255 * (i & 2), 255 * (i & 1))) for i in range(1, 8)]
red = bsk.Material(color=(255, 0, 0))
blue = bsk.Material(color=(0, 0, 255))

radius = 0.5

nodes = [scene.add_node(
    position=[random.uniform(-radius, radius) for __ in range(3)], 
    scale=[random.uniform(0.5, 2) for _ in range(3)],
    rotation=[random.uniform(-radius, radius), random.uniform(-3, radius), random.uniform(-radius, radius)], 
    mesh=random.choice(meshes), 
    material=blue,
    collisions=True
) for _ in range(25)]

is_pressed = False

while engine.running:    
    keys = pg.key.get_pressed()
    if keys[pg.K_q] and not is_pressed: 
        is_pressed = True
        
    
    if not keys[pg.K_q] and is_pressed: 
        is_pressed = False
        possible = scene.collider_handler.resolve_broad_collisions()
        
        for node in nodes: node.material = blue
        for collision in possible:
            node1 = collision[0].node
            node2 = collision[1].node
            node1.material = red
            node2.material = red
        
        scene.collider_handler.resolve_narrow_collisions(possible)

    engine.update()