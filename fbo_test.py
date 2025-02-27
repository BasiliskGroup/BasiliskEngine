import basilisk as bsk

engine = bsk.Engine()
scene = bsk.Scene(engine)

scene.add(bsk.Node())

fbo = bsk.Framebuffer(engine, n_color_attachments=2)
fbo2 = bsk.Framebuffer(engine)

while engine.running:
    scene.update(render=False)

    scene.render(fbo)
    fbo.render(fbo2)
    fbo2.render()
    
    engine.update()