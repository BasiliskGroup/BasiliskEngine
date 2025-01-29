import basilisk as bsk
from random import uniform
import glm

engine = bsk.Engine()
scene  = bsk.Scene()
engine.scene = scene

print(len(engine.cube.indices))

sphere = bsk.Mesh('tests/sphere.obj')
red = bsk.Material(color=(255, 0, 0))
blue = bsk.Material(color=(0, 0, 255))

for _ in range(1000):
    scene.add(bsk.Node(
        position=[uniform(-100, 100) for _ in range(3)],
        scale=[uniform(0.5, 2) for _ in range(3)],
        rotation=[uniform(0, glm.pi()) for _ in range(3)],
        collisions=True,
        material=blue,
        mesh=sphere,
        static=True
    ))

left_pressed = False

while engine.running:
    
    if engine.mouse.left_down: left_pressed = True
    elif left_pressed: 
        
        node, intersection = engine.scene.raycast_mouse((engine.mouse.x, engine.mouse.y), has_collisions=True)
        left_pressed = False
        
        if not node: continue
        
        print(intersection)
        
        scene.add(bsk.Node(
            position = intersection,
            material=red,
            scale=(0.1, 0.1, 0.1)
        ))
    
    engine.update()