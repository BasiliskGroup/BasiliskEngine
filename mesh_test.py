import basilisk as bsk
import numpy as np

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene


mesh_data = np.array([[0, 0, 0],
                      [0, 0, 10],
                      [10, 0, 0]])
mesh = bsk.Mesh(mesh_data)

scene.add_node(mesh=mesh)

while engine.running:
    engine.update()