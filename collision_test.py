import basilisk as bsk
import pygame as pg
import random

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

# meshes
cube_mesh = bsk.cube
sphere_mesh = bsk.Mesh('tests/sphere.obj')

meshes = [sphere_mesh]

# materials
mud = bsk.Image("tests/mud.png")
mud_normal = bsk.Image("tests/mud_normal.png")
mud_mtl = bsk.Material(texture=mud, normal=mud_normal)
materials = [bsk.Material(color=(255 * (i & 4), 255 * (i & 2), 255 * (i & 1))) for i in range(1, 8)]
red = bsk.Material(color=(255, 0, 0))
blue = bsk.Material(color=(0, 0, 255))

is_pressed = False
radius = 0.5

nodes = [scene.add_node(
    position=[random.uniform(-radius, radius) for __ in range(3)], 
    scale=[random.uniform(0.5, 2) for _ in range(3)],
    rotation=[random.uniform(-radius, radius), random.uniform(-3, radius), random.uniform(-radius, radius)], 
    mesh=random.choice(meshes), 
    material=materials[_ % 2],
    collisions=True
) for _ in range(2)]

# print(len(possible))
is_pressed = False

while engine.running:

    # control one object, idk which one it is good luck
    keys = pg.key.get_pressed()
    if keys[pg.K_q] and not is_pressed: is_pressed = True
    if not keys[pg.K_q] and is_pressed: 
        
        possible = scene.collider_handler.resolve_broad_collisions()
        print(len(possible))
        collided = scene.collider_handler.resolve_narrow_collisions(possible)
        print(len(collided))
        print()
        
        for collision in collided:
            node1 = collision[0]
            node2 = collision[1]
            vec = collision[2]
            
            scene.add_node(
                position=node1.position - vec, 
                scale=[0.1, 0.1, 0.1],
                mesh=random.choice(meshes), 
                material=materials[5],
            )

        is_pressed = False
    
    # set color of colliding objects
    # for collision in possible:
    #     collider1 = collision[0]
    #     collider2 = collision[1]
        
    #     if not scene.collider_handler.collide_obb_obb_decision(collider1, collider2): continue
        
    #     collider1.node.material = red
    #     collider2.node.material = red
    
    engine.update()