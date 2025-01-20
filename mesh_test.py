import basilisk as bsk
import numpy as np

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

def h(x, y):
    h1 = (np.sin(x/2) + np.cos(y/2)) * 1
    h2 = (np.sin(x/4) + np.cos(y/4)) * 2
    h3 = (np.sin(x/8) + np.cos(y/8)) * 3
    return h1 + h2 + h3

def get_data():
    mesh_data = []

    for x in range(-30, 30):
        for y in range(-30, 30):
            p1 = [x    , h(x    , y    ), y    ]
            p2 = [x + 1, h(x + 1, y    ), y    ]
            p3 = [x    , h(x    , y + 1), y + 1]
            p4 = [x + 1, h(x + 1, y + 1), y + 1]

            mesh_data.extend([p1, p3, p2, p2, p3, p4])

    return np.array(mesh_data)

mesh = bsk.Mesh(get_data())
scene.add_node(mesh=mesh)

while engine.running:
    engine.update()