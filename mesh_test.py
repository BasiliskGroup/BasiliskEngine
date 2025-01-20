import basilisk as bsk
import numpy as np

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene


height_map = [[0 for x in range(10)] for y in range(10)]

# [[x    , 0, y    ],
#  [x + 1, 0, y    ],
#  [x    , 0, y + 1]]

mesh_data = []
for x in range(10):
    for y in range(10):
        mesh_data.extend([[x, 0, y], [x + 1, 0, y], [x, 0, y + 1]])

mesh_data = np.array(mesh_data)

mesh = bsk.Mesh(mesh_data)

scene.add_node(mesh=mesh)

while engine.running:
    engine.update()