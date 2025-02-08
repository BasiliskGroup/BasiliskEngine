import basilisk as bsk
import random

engine = bsk.Engine()
scene  = bsk.Scene()
engine.scene = scene

parent = bsk.Node(
    scale = (1, 1, 3)
)
child  = bsk.Node(
    position = (1, 0, 0),
    scale = (0.5, 0.5, 0.5)
)
grandchild = bsk.Node(
    position = (1.5, 0, 0),
    scale = (2, 0.2, 0.2)
)

child.add(grandchild)

scene.add(parent)
parent.add(child)

while engine.running:
        
    # parent.rotation = scene.camera.rotation
    parent.position += (0.003, 0, 0)
    
    engine.update()
    
