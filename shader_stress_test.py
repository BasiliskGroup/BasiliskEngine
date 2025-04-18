import basilisk as bsk
import random

engine = bsk.Engine(title=None)
scene = bsk.Scene(engine)

default_shader = bsk.Shader(engine)
normal_shader  = bsk.Shader(engine, vert='basilisk/shaders/normal.vert', frag='basilisk/shaders/normal.frag')
geom_shader  = bsk.Shader(engine, vert='basilisk/shaders/geometry.vert', frag='basilisk/shaders/geometry.frag')

shaders = [None, default_shader, normal_shader, geom_shader]

n = 10
d = 5

for x in range(-n, n):
    for y in range(-n, n):
        for z in range(-n, n):
            scene.add(bsk.Node(position=(x * d, y * d, z * d), shader=shaders[random.randint(0, 3)]))

while engine.running:
    scene.update()
    if engine.keys[bsk.pg.K_1]:
        scene.shader = default_shader
    if engine.keys[bsk.pg.K_2]:
        scene.shader = normal_shader
    if engine.keys[bsk.pg.K_3]:
        scene.shader = geom_shader
    engine.update()