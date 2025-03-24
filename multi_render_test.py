import basilisk as bsk

engine = bsk.Engine()
scene = bsk.Scene(engine)

shader = bsk.Shader(engine, 'tests/shaders/multi_render_test.vert', 'tests/shaders/multi_render_test.frag')
fbo = bsk.Framebuffer(engine, n_color_attachments=6)
ugh = bsk.Framebuffer(engine, n_color_attachments=2)
test2 = bsk.Framebuffer(engine, n_color_attachments=3)
scene.add(bsk.Node(shader=shader))

show = 0

while engine.running:

    if engine.keys[bsk.pg.K_1]: show = 0
    if engine.keys[bsk.pg.K_2]: show = 1
    if engine.keys[bsk.pg.K_3]: show = 2
    if engine.keys[bsk.pg.K_4]: show = 3
    if engine.keys[bsk.pg.K_5]: show = 4
    if engine.keys[bsk.pg.K_6]: show = 5
    if engine.keys[bsk.pg.K_7]: show = 6

    scene.update(render=False)
    scene.render(fbo)
    fbo.render(color_attachment=show)
    engine.update(render=False)
