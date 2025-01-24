import basilisk as bsk
import numpy as np
import glm
import time

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

# engine.shader = bsk.Shader(engine, vert='basilisk/shaders/normal.vert', frag='basilisk/shaders/normal.frag')

texture = bsk.Image('tests/grass.jpg')
normal = bsk.Image('tests/grass_normal.png')
roughness = bsk.Image('tests/grass_roughness.jpg')
ao = bsk.Image('tests/grass_ao.jpg')
mtl = bsk.Material(texture=texture, normal=normal, roughness_map=roughness, ao_map=ao, roughness=0.6, subsurface=0.2, sheen=1.0, sheen_tint=1.0, anisotropic=0.6, specular=0.4, metallicness=0.0, specular_tint=0.55, clearcoat=1.0, clearcoat_gloss=0.465)

def h(x, y):
    h1 = (np.sin(x/2) + np.cos(y/2)) * 1
    h2 = (np.sin(x/4) + np.cos(y/4)) * 2
    h3 = (np.sin(x/8) + np.cos(y/8)) * 3
    return h1 + h2 + h3

def get_height_array(size, offset=(0, 0)):
    return [[h(x + offset[0], y + offset[1]) for x in range(size)] for y in range(size)]

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

def get_data(size=30, offset=(0, 0)):
    height_array = get_height_array(size+2, offset)
    normal_array = get_normal_array(size, height_array)
    mesh_data = []

    for x in range(size):
        for y in range(size):
            n1 = normal_array[x    ][y    ]
            n2 = normal_array[x + 1][y    ]
            n3 = normal_array[x    ][y + 1]
            n4 = normal_array[x + 1][y + 1]

            texture_scale = 25
            tx_1, ty_1 = x / texture_scale, y / texture_scale
            tx_2, ty_2 = (x + 1) / texture_scale, (y + 1) / texture_scale

            p1 = [x    , height_array[x    ][y    ], y    , tx_1, ty_1, *n1]
            p2 = [x + 1, height_array[x + 1][y    ], y    , tx_2, ty_1, *n2]
            p3 = [x    , height_array[x    ][y + 1], y + 1, tx_1, ty_2, *n3]
            p4 = [x + 1, height_array[x + 1][y + 1], y + 1, tx_2, ty_2, *n4]

            mesh_data.extend([p1, p3, p2, p2, p3, p4])

    return np.array(mesh_data)


t1 = time.time()
mesh = bsk.Mesh(get_data(size=16))
t2 = time.time()

print(f'Mesh time: {t2 - t1}')

scene.add_node(position=(0, -10, 0), mesh=mesh, material=mtl)

while engine.running:
    engine.update()