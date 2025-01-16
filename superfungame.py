import basilisk as bsk

engine = bsk.Engine()
scene  = bsk.Scene()
engine.scene = scene

scene.add_node(position=(0, -3, 0), scale=(10, 1, 10), collisions=True)
box = scene.add_node(position=(3, 2, 3), scale=(1, 1, 1), collisions=True, physics=True, mass=30)

player = scene.add_node(position=(0, 0, 0), scale=(.5, 1, .5))
# camera = bsk.FollowCamera(player)
# scene.camera = camera

def move_player():
    look_vector = scene.camera.forward.xz
    strafe_vector = scene.camera.right.xz

    walk_magnitude = (engine.keys[bsk.pg.K_w] - engine.keys[bsk.pg.K_s]) * engine.delta_time * 5
    strafe_magnitude = (engine.keys[bsk.pg.K_d] - engine.keys[bsk.pg.K_a]) * engine.delta_time * 5

    player.x += (walk_magnitude * look_vector.x) + (strafe_magnitude * strafe_vector.x)
    player.z += (walk_magnitude * look_vector.y) + (strafe_magnitude * strafe_vector.y)

while engine.running:
    move_player()
    
    engine.update()
