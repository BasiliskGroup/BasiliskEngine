import basilisk as bsk

engine = bsk.Engine()
scene = bsk.Scene(engine)

shader = bsk.Shader(engine, 'tests/shaders/multi_render_test.vert', 'tests/shaders/multi_render_test.frag')
fbo = bsk.Framebuffer(engine, n_color_attachments=6)
scene.add(bsk.Node(shader=shader))

while engine.running:

    if engine.keys[bsk.pg.K_1]: fbo.show = 0
    if engine.keys[bsk.pg.K_2]: fbo.show = 1
    if engine.keys[bsk.pg.K_3]: fbo.show = 2
    if engine.keys[bsk.pg.K_4]: fbo.show = 3
    if engine.keys[bsk.pg.K_5]: fbo.show = 4
    if engine.keys[bsk.pg.K_6]: fbo.show = 5

    scene.update(render=False)
    scene.render(fbo)
    fbo.render()
    engine.update()