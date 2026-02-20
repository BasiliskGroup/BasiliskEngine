"""
Using a custom shader
"""

import basilisk as bsk

engine = bsk.Engine()

shader = bsk.Shader("shaders/test.vert", "shaders/test.frag")
scene = bsk.Scene(engine, shader)

mesh = bsk.Mesh("models/sphere.obj")
node = bsk.Node(scene, mesh)

while engine.is_running():
    engine.update()
    scene.update()
    scene.render()
    engine.render()