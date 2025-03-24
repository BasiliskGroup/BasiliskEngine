import basilisk as bsk

engine = bsk.Engine(title=None)
scene = bsk.Scene(engine)
scene.ctx.multisample = True

scene.add(bsk.Node())

fbo1 = bsk.Framebuffer(engine)
fbo2 = bsk.Framebuffer(engine)


while engine.running:
    scene.update(render=False)
    scene.render()
    scene.frame.render(fbo1)
    fbo1.render()
    engine.update(render=False)