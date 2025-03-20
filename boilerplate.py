import basilisk as bsk

engine = bsk.Engine()
scene = bsk.Scene(engine)

scene.add(bsk.Node())

while engine.running:
    scene.update()

    # scene.frame.input_buffer.render(color_attachment=1)

    engine.update()