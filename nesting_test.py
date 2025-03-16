import basilisk as bsk
import random
import glm

engine = bsk.Engine()
scene  = bsk.Scene(engine)

parent = bsk.Node(
    scale = (3, 1, 3),
    rotation = glm.angleAxis(random.uniform(-1, 1), glm.normalize([random.uniform(-1, 1) for _ in range(3)]))
)
child  = bsk.Node(
    position = (3, 0, 0),
    scale = (0.5, 0.5, 0.5),
    # relative_scale=False,
    # relative_rotation= False
    # relative_position=False
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
        
    parent.scale += 0.001
    parent.position -= 0.001
    if engine.keys[bsk.pg.K_e]: child.rotation = glm.normalize([random.uniform(-1, 1) for _ in range(3)])
    
    scene.update()
    engine.update()
    
