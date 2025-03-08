import basilisk as bsk

engine = bsk.Engine()
scene = bsk.Scene(engine)

shader = bsk.Shader(engine, 'tests/shaders/image_test.vert', 'tests/shaders/image_test.frag')
img = bsk.Image('tests/floor_albedo.png')

shader.bind(img, 'image')

scene.add(bsk.Node(shader=shader))

while engine.running:
    scene.update()
    engine.update()