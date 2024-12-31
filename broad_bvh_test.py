import basilisk as bsk
import pygame as pg
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

aabb_edges = []
is_pressed = False

radius = 20

while engine.running:
    keys = pg.key.get_pressed()
    if keys[pg.K_k] and not is_pressed: is_pressed = True
    if not keys[pg.K_k] and is_pressed: 
        is_pressed = False
        
        for i in range(5):
            node = scene.add_node(
                position=[random.uniform(-radius, radius), random.uniform(-3, -radius), random.uniform(-radius, radius)], 
                scale=[random.uniform(0.5, 2) for _ in range(3)],
                rotation=[random.uniform(-radius, radius), random.uniform(-3, -radius), random.uniform(-radius, radius)], 
                mesh=cube_mesh, 
                material=materials[i % 6],
                collisions=True
            )
        
        # vertices = node.collider.obb_points
    
        # for vertex in vertices:
        #     scene.add_node(
        #         position = vertex,
        #         scale = (0.1, 0.1, 0.1),
        #         material = materials[6],
        #         mesh=cube_mesh
        #     )
        
        # remove old aabbs
        
        for edge in aabb_edges:
            scene.node_handler.remove(edge)
            
        aabb_edges = []
        
        # add new aabbs
        aabbs = scene.collider_handler.bvh.get_all_aabbs()
        for (top_right, bottom_left, layer) in aabbs:

            x1, y1, z1 = top_right
            x2, y2, z2 = bottom_left
                
            vertices = [glm.vec3(x, y, z) for z in (z1, z2) for y in (y1, y2) for x in (x1, x2)]
            
            edges = [
                (vertices[0], vertices[1]),
                (vertices[0], vertices[2]),
                (vertices[0], vertices[4]),
                (vertices[1], vertices[3]),
                (vertices[1], vertices[5]),
                (vertices[2], vertices[3]),
                (vertices[2], vertices[6]),
                (vertices[3], vertices[7]),
                (vertices[4], vertices[5]),
                (vertices[4], vertices[6]),
                (vertices[5], vertices[7]),
                (vertices[6], vertices[7]),
            ]
            
            for edge in edges:
                center = (edge[1] + edge[0]) / 2
                dimensions = glm.abs(edge[1] - edge[0]) + glm.vec3(0.02)
                dimensions /= 2
                aabb_edge = scene.add_node(
                    position = center,
                    scale = dimensions,
                    material=materials[layer % 6],
                    mesh=cube_mesh
                )
                aabb_edges.append(aabb_edge)
                
            # obb axes
            rot_mat = glm.transpose(glm.mat3_cast(node.rotation))
            for i in range(3):
                scene.add_node(
                    position = node.position + rot_mat[i] * 3,
                    scale = (0.1, 0.1, 0.1),
                    rotation = node.rotation,
                    material=materials[6],
                    mesh=cube_mesh
                )
    
    engine.update()