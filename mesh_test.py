import basilisk as bsk
import numpy as np
import glm

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

mtl = bsk.Material(color=(50, 200, 100), roughness= .75, subsurface=.5)

def h(x, y):
    h1 = (np.sin(x/2) + np.cos(y/2)) * 1
    h2 = (np.sin(x/4) + np.cos(y/4)) * 2
    h3 = (np.sin(x/8) + np.cos(y/8)) * 3
    return h1 + h2 + h3

def get_height_array(size):
    return [[h(x, y) for x in range(size)] for y in range(size)]

def get_normal_array(size, height_array):
    normal_array = np.zeros(shape=(size + 1, size + 1, 3))
    for x in range(size + 1):
        for y in range(size + 1):
            total_norm = glm.vec3(0, 0, 0)

            total_norm -= glm.cross(glm.vec3(x - (x + 1), height_array[x][y] - height_array[x + 1][y], 0), glm.vec3(0, height_array[x][y] - height_array[x][y + 1], y - (y  + 1)))
            total_norm += glm.cross(glm.vec3(x - (x + 1), height_array[x][y] - height_array[x + 1][y], 0), glm.vec3(0, height_array[x][y] - height_array[x][y - 1], y - (y  - 1)))
            total_norm -= glm.cross(glm.vec3(x - (x - 1), height_array[x][y] - height_array[x - 1][y], 0), glm.vec3(0, height_array[x][y] - height_array[x][y - 1], y - (y  - 1)))
            total_norm += glm.cross(glm.vec3(x - (x - 1), height_array[x][y] - height_array[x - 1][y], 0), glm.vec3(0, height_array[x][y] - height_array[x][y + 1], y - (y  + 1)))
            
            total_norm /= 4

            normal_array[x][y] = glm.normalize(total_norm).xyz

    return normal_array

def get_data(size=100):
    height_array = get_height_array(size+2)
    normal_array = get_normal_array(size, height_array)
    mesh_data = []

    for x in range(size):
        for y in range(size):
            n1 = normal_array[x    ][y    ]
            n2 = normal_array[x + 1][y    ]
            n3 = normal_array[x    ][y + 1]
            n4 = normal_array[x + 1][y + 1]

            p1 = [x    , height_array[x    ][y    ], y    , *n1]
            p2 = [x + 1, height_array[x + 1][y    ], y    , *n2]
            p3 = [x    , height_array[x    ][y + 1], y + 1, *n3]
            p4 = [x + 1, height_array[x + 1][y + 1], y + 1, *n4]

            mesh_data.extend([p1, p3, p2, p2, p3, p4])

    return np.array(mesh_data)

mesh = bsk.Mesh(get_data())
scene.add_node(mesh=mesh, material=mtl)

while engine.running:
    engine.update()