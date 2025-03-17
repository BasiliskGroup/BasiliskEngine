import basilisk as bsk
import moderngl as mgl

engine = bsk.Engine()
scene = bsk.Scene(engine)
engine.config.bloom_enabled = False

crt = bsk.PostProcess(engine, 'basilisk/shaders/crt.frag')
mtl = bsk.Material(emissive_color=(255 * 8, 255 * 8, 0))
scene.add(bsk.Node(material=mtl))


myfbo = bsk.Framebuffer(engine)

while engine.running:
    scene.update()
    engine.frame.bloom.render()

    crt.apply([('screenTexture', engine.frame)], engine.frame.ping_pong_buffer)


    temp = engine.frame.framebuffer
    engine.frame.framebuffer = engine.frame.ping_pong_buffer
    engine.frame.ping_pong_buffer = temp

    engine.update()
