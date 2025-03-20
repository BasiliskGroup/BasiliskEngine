import basilisk as bsk
import glm

engine = bsk.Engine()
scene = bsk.Scene(engine)


scene.add(bsk.Node(position=(0, 0, -5), scale=(10, 10, 1)))
scene.add(bsk.Node())

edge_detect_shader = bsk.Shader(engine, vert='basilisk/shaders/frame.vert', frag='tests/shaders/edge.frag')
edge_detect_fbo = bsk.Framebuffer(engine, shader=edge_detect_shader)

edges = bsk.Framebuffer(engine)

while engine.running:
    scene.update()

    edge_detect_fbo.bind(scene.frame.input_buffer.depth, 'depthTexture', 0)
    edge_detect_fbo.bind(scene.frame.input_buffer.color_attachments[2], 'normalTexture', 1)
    edge_detect_fbo.render(edges, auto_bind=False)

    edges.render()

    # scene.frame.input_buffer.render(color_attachment=2)

    engine.update(render=False)