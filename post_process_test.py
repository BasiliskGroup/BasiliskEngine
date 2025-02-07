import basilisk as bsk

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

crt = bsk.PostProcess(engine, "basilisk/shaders/crt.frag")
scene.frame.post_processes.append(crt)

scene.add(bsk.Node())

while engine.running:
    engine.update()
