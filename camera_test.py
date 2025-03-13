import basilisk as bsk
import random
import glm

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene
camera = scene.camera

for _ in range(100):
    scene.add(bsk.Node(position = [random.uniform(-100, 100) for _ in range(3)]))

def key_down(key):
    return engine.keys[key] and not engine.previous_keys[key]

def actions():
    if key_down(bsk.pg.K_RIGHT): camera.yaw += 1
    if key_down(bsk.pg.K_LEFT): camera.yaw -= 1
    if key_down(bsk.pg.K_UP): camera.pitch += 1
    if key_down(bsk.pg.K_DOWN): camera.pitch -= 1
    if key_down(bsk.pg.K_p): camera.pitch = 0
    if key_down(bsk.pg.K_y): camera.yaw = 270

while engine.running:
    
    actions()
    print(camera.yaw, camera.pitch, camera.UP, camera.forward, camera.right)
    
    engine.update()