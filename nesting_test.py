import basilisk as bsk
import random

engine = bsk.Engine()
scene  = bsk.Scene(engine)

parent = bsk.Node(
    scale = (3, 1, 3),
)
child  = bsk.Node(
    position = (3, 0, 0),
    scale = (0.5, 0.5, 0.5)
)
grandchild = bsk.Node(
    position = (1.5, 0, 0),
    scale = (2, 0.2, 0.2)
)

# child.add(grandchild)
scene.add(parent)
parent.add(child)
# scene.add(grandchild)
# parent.add(child)

while engine.running:
        
    parent.rotation = scene.camera.rotation
    # parent.position += (0.003, 0, 0)
    
    scene.update()
    engine.update()
    
