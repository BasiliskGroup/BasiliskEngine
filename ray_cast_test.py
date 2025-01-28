import basilisk as bsk
import glm

engine = bsk.Engine()
scene  = bsk.Scene()
engine.scene = scene

red = bsk.Material(color=(255, 0, 0))

scene.add(bsk.Node(
    position=(4, 1, 1),
    scale=(2, 3, 4),
    rotation=(0.5, 2, 3),
    collisions=True
))

scene.add(bsk.Node())

left_pressed = False

while engine.running:
    
    if engine.mouse.left_down: left_pressed = True
    elif left_pressed: 
        
        node, intersection = engine.scene.raycast_mouse((engine.mouse.x, engine.mouse.y), has_collisions=True) #  a
        left_pressed = False
        
        if not node: continue
        
        print(intersection)
        
        scene.add_node(
            position = intersection,
            material=red,
            scale=(0.1, 0.1, 0.1)
        )
    
    engine.update()