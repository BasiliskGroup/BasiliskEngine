import basilisk as bsk
import numpy as np

engine = bsk.Engine()
scene = bsk.Scene(engine)

scene.camera = bsk.StaticCamera()
scene.camera.position = (5, 3, 20)

# Create a shader with a custom format
test_shader = bsk.Shader(engine, 'tests/shaders/custom_format.vert', 'tests/shaders/custom_format.frag')

# Create a mesh and specify that it is custom
c_val = 1
quad = np.array([[0, 0, 0, c_val], [0, 0, 10, 0], [10, 0, 10, c_val],
                    [0, 0, 0, c_val], [10, 0, 10, c_val], [10, 0, 0, c_val]])
mesh = bsk.Mesh(quad, custom_format=True)

node = bsk.Node(mesh=mesh, shader=test_shader)
scene.add(node)

nodes = [node]

cube   = bsk.Mesh('tests/cube.obj')
monkey = bsk.Mesh('tests/monkey.obj')

while engine.running:

    scene.update()
    dt = engine.dt

    if engine.keys[bsk.pg.K_1] and not node:
        node = scene.add(bsk.Node(mesh=mesh, shader=test_shader))
        nodes.append(node)
    if engine.keys[bsk.pg.K_2]:
        node = scene.remove(node)
    if engine.keys[bsk.pg.K_3] and node:
        node.x += 5 * dt
    if engine.keys[bsk.pg.K_4] and node:
        node.x -= 5 * dt
    if engine.keys[bsk.pg.K_5] and node:
        node.position.x += 5 * dt
    if engine.keys[bsk.pg.K_6] and node:
        node.scale.x += 5 * dt
    if engine.keys[bsk.pg.K_7] and node:
        node.rotation.x += 5 * dt
    if engine.keys[bsk.pg.K_8] and node:
        node.mesh = monkey
    if engine.keys[bsk.pg.K_9]:
        nodes.extend(scene.add(*(bsk.Node(position=(i, 3, 0)) for i in range(-10, 10, 4))))
    if engine.keys[bsk.pg.K_0]:
        scene.remove(*nodes)
        nodes = []

    engine.update()