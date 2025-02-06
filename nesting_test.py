import basilisk as bsk
import random

engine = bsk.Engine()
scene  = bsk.Scene()
engine.scene = scene

parent = bsk.Node(
    velocity=(1, 0, 0),
    rotational_velocity=(0.1, 0.1, 0)
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

copy = parent.deep_copy()

copy.velocity = (0, 0, 0)

scene.add(copy)

def actions():
    
    if engine.keys[bsk.pg.K_p] and not engine.previous_keys[bsk.pg.K_p]:
        print(len(scene.node_handler.get_all()))

did = False
while engine.running:
    
    actions()
    
    if not did:
        parent.add(child)
        did = True
    parent.scale += [random.uniform(-0.01, 0.01) for i in range(3)]
    engine.update()
    
