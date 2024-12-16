import basilisk as bsk
from math import cos, sin, pi
import glm
import time
import random

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

# start = time.time()
cube_mesh = bsk.Mesh('tests/cube.obj')
# cube = time.time()
sphere_mesh = bsk.Mesh('tests/sphere.obj')
# sphere = time.time()
lucy_mesh = bsk.Mesh('tests/monkey.obj')
# lucy = time.time()
# for m in ['closet_001', 'cylinder', 'cube', 'sphere', 'fridge_001', 'monkey']:
#     mesh = bsk.Mesh(f'tests/{m}.obj')
#     print(m, len(mesh.points))

# print('cube:', cube - start)
# print('sphere:', sphere - cube)
# print('lucy:', lucy - sphere)

# mud = bsk.Image("tests/mud.png")
# mud_normal = bsk.Image("tests/mud_normal.png")
mud_mtl = bsk.Material(color=(255, 255, 255))
materials = [bsk.Material(color=(255 * (i & 4), 255 * (i & 2), 255 * (i & 1))) for i in range(1, 7)]

scene.add_node(
    # position=(0, 0, -1), 
    # scale=(1, 2, 3),
    mesh=lucy_mesh, 
    material=mud_mtl,
    # velocity=(0, 0.1, 0),
    # rotational_velocity=(0, 1, 0),
    # physics=True,
    # mass=1
)

old_time = 0
new_time = 0
trials = 10000

for i in range(trials):
    vec = glm.vec3([random.uniform(-10, 10) for _ in range(3)])
    start = time.time()
    old_vec = lucy_mesh.get_best_dot(vec)
    old = time.time()
    bvh_vec = lucy_mesh.get_best_dot(vec)
    new = time.time()
    
    old_time += old - start
    new_time += new - old
    
    if old_vec != bvh_vec: print(old_vec, bvh_vec, 'difference')
    
print('old', old_time / trials)
print('new', new_time / trials)

# scene.add_node(
#     mesh=cube_mesh,
#     material=materials[2], # green
#     scale=(0.03, 0.03, 0.03),
#     position=old_vec
# )

# scene.add_node(
#     mesh=cube_mesh,
#     material=materials[4], # pink
#     scale=(0.03, 0.03, 0.03),
#     position=bvh_vec
# )
    
# for i in range(10):
#     scene.add_node(
#         mesh=cube_mesh,
#         material=materials[5], # yellow
#         scale=(0.01, 0.01, 0.01),
#         position=vec / 8 * (i + 1)
#     )

aabbs = lucy_mesh.bvh.get_all_aabbs()
print('aabbs: ', len(aabbs))
for aabb in aabbs:
    
    if aabb[2] != 3: continue
    
    x1, y1, z1 = aabb[1]
    x2, y2, z2 = aabb[0]
    
    vertices = [glm.vec3(x, y, z) for z in (z1, z2) for y in (y1, y2) for x in (x1, x2)]
    
    edges = [
        (vertices[0], vertices[1]),
        (vertices[0], vertices[2]),
        (vertices[0], vertices[4]),
        (vertices[1], vertices[3]),
        (vertices[1], vertices[5]),
        (vertices[2], vertices[3]),
        (vertices[2], vertices[6]),
        (vertices[3], vertices[7]),
        (vertices[4], vertices[5]),
        (vertices[4], vertices[6]),
        (vertices[5], vertices[7]),
        (vertices[6], vertices[7]),
    ]
    
    for edge in edges:
        center = (edge[1] + edge[0]) / 2
        dimensions = glm.abs(edge[1] - edge[0]) + glm.vec3(0.01)
        dimensions /= 2
        scene.add_node(
            position = center,
            scale = dimensions,
            material=materials[aabb[2] % 6],
            mesh=cube_mesh
        )

while engine.running:
    engine.update()