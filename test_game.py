import basilisk as bsk
import pygame as pg

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

floor = scene.add_node(position=(0, -3, 0), scale=(10, 1, 10), collisions=True)
player = scene.add_node(position=(0, 0, 0), scale=(.5, 1, .5))

camera = bsk.FollowCamera(player)
# camera = bsk.OrbitCamera(player, distance=7)
scene.camera = camera

def move_player():
    look_vector = camera.forward.xz
    strafe_vector = camera.right.xz

    walk_magnitude = (engine.keys[pg.K_w] - engine.keys[pg.K_s]) * engine.delta_time * 5
    strafe_magnitude = (engine.keys[pg.K_d] - engine.keys[pg.K_a]) * engine.delta_time * 5

    player.x += (walk_magnitude * look_vector.x) + (strafe_magnitude * strafe_vector.x)
    player.z += (walk_magnitude * look_vector.y) + (strafe_magnitude * strafe_vector.y)

def draw_crosshair():
    bsk.draw.circle(engine, (0, 0, 0), (engine.win_size[0] // 2, engine.win_size[1] // 2), 4)
    bsk.draw.circle(engine, (255, 255, 255), (engine.win_size[0] // 2, engine.win_size[1] // 2), 3)

def shoot():
    if not engine.mouse.click: return
    
    pos = player.position + camera.right / 4
    hit_pos = player.position + camera.forward * 25
    vel = (hit_pos - pos) * 2
    
    projectile = scene.add_node(pos, scale=(.1, .1, .1), physics=True, collisions=True, velocity=vel)

while engine.running:
    move_player()
    draw_crosshair()
    shoot()

    engine.update()