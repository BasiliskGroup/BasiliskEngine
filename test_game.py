import basilisk as bsk

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

wood_texture = bsk.Image('tests/floor_albedo.png')
wood_normal = bsk.Image('tests/floor_normal.png')
brick_texture = bsk.Image('tests/brick.png')
brick_normal = bsk.Image('tests/brick_normal.png')

wood = bsk.Material(texture=wood_texture, normal=wood_normal,
                    roughness=.4, subsurface=0.0, sheen=.9, metallicness=.15, anisotropic=0.15, specular=1.0, clearcoat=.8, clearcoat_gloss=.75)
brick = bsk.Material(texture=brick_texture, normal=brick_normal, roughness=1.0, clearcoat=1.0, specular=1.5, specular_tint=.25)
bullet = bsk.Material(color=(255, 100, 100))


floor = scene.add_node(position=(0, -3, 0), scale=(10, 1, 10), collisions=True, material=wood)
player = scene.add_node(position=(0, 0, 0), scale=(.5, 1, .5))
box = scene.add_node(position=(3, 0, 3), scale=(1, 1, 1), collisions=True, physics=True, mass=30, material=brick)

camera = bsk.FollowCamera(player)
scene.camera = camera

def move_player():
    look_vector = camera.forward.xz
    strafe_vector = camera.right.xz

    walk_magnitude = (engine.keys[bsk.pg.K_w] - engine.keys[bsk.pg.K_s]) * engine.delta_time * 5
    strafe_magnitude = (engine.keys[bsk.pg.K_d] - engine.keys[bsk.pg.K_a]) * engine.delta_time * 5

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
    
    scene.add_node(pos, scale=(.1, .1, .1), physics=True, collisions=True, velocity=vel, material=bullet)

while engine.running:
    move_player()
    draw_crosshair()
    shoot()

    engine.update()