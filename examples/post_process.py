"""
Using a post-processing shader
"""

import basilisk as bsk

engine = bsk.Engine()
scene = bsk.Scene(engine)

mesh = bsk.Mesh("models/sphere.obj")
node = bsk.Node(scene, mesh)

shader = bsk.Shader("shaders/frame.vert", "shaders/post.frag")
frame = bsk.Frame(engine, shader, 160, 160)
frame.set_filter_nearest()

while engine.is_running():
    engine.update()
    scene.update()

    frame.clear()
    frame.use()
    scene.render()

    engine.get_frame().use()
    frame.render()
    engine.render()