import basilisk as bsk
import moderngl as mgl
import pygame as pg

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene


fbo = bsk.Framebuffer(engine, resolution_scale=.1, filter=(mgl.NEAREST, mgl.NEAREST))

# crt = bsk.PostProcess(engine, "basilisk/shaders/crt.frag")
# downscale = bsk.PostProcess(engine, size=(80, 80), filter=(mgl.NEAREST, mgl.NEAREST))
# scene.add(downscale)


scene.add(bsk.Node())

while engine.running:
    engine.update(render=False)

    scene.render(fbo)
    engine.ctx.screen.use()
    engine.ctx.clear()
    fbo.render()
    pg.display.flip()
