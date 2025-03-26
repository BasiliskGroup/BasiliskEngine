import basilisk as bsk
import glm

engine = bsk.Engine()
scene = bsk.Scene(engine)
scene.sky = None
camera = scene.camera

node = bsk.Node(scale = glm.vec3(0.1))
scene.add(node)

while engine.running:
    node.position = camera.position + camera.forward * 2
    node.rotation = glm.conjugate(camera.rotation)
    scene.update()
    engine.update()
    