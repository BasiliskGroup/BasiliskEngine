import basilisk as bsk
import glm

engine = bsk.Engine()
scene  = bsk.Scene(engine)

node = bsk.Node(
    
)

scene.add(node)

def move():
    node.position.x += 0.01
    node.scale.y += 0.01
    node.rotation.z += 0.01

while engine.running:
    
    move()
    
    scene.update()
    engine.update()