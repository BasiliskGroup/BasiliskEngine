import basilisk as bsk
import numpy as np

engine = bsk.Engine()
scene = bsk.Scene(engine)
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

while engine.running:
    scene.update()
    engine.update()