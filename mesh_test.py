import basilisk as bsk
import numpy as np

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene


height_map = [[0 for x in range(10)] for y in range(10)]

# [[x    , 0, y    ],
#  [x + 1, 0, y    ],
#  [x    , 0, y + 1]]

def get_data():
    mesh_data = []

    def h(x, y):
        return np.sin(x) + np.cos(y)

    for x in range(-10, 10):
        for y in range(-10, 10):
            p1 = [x    , h(x    , y    ), y    ]
            p2 = [x + 1, h(x + 1, y    ), y    ]
            p3 = [x    , h(x    , y + 1), y + 1]
            p4 = [x + 1, h(x + 1, y + 1), y + 1]

            mesh_data.extend([p1, p3, p2, p2, p3, p4])

    return mesh_data

mesh_data = np.array(get_data())

mesh = bsk.Mesh(mesh_data)

scene.add_node(mesh=mesh)

while engine.running:
    engine.update()