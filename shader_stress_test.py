import basilisk as bsk
import random

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

default_shader = bsk.Shader(engine)
normal_shader  = bsk.Shader(engine, vert='basilisk/shaders/normal.vert', frag='basilisk/shaders/normal.frag')
geom_shader  = bsk.Shader(engine, vert='basilisk/shaders/geometry.vert', frag='basilisk/shaders/geometry.frag')

shaders = [None, default_shader, normal_shader, geom_shader]

n = 10
d = 5

for x in range(-n, n):
    for y in range(-n, n):
        for z in range(-n, n):
            scene.add_node(position=(x * d, y * d, z * d), shader=shaders[random.randint(0, 3)])

while engine.running:
    if engine.keys[bsk.pg.K_1] and not engine.previous_keys[bsk.pg.K_1]:
        engine.shader = default_shader
    if engine.keys[bsk.pg.K_2] and not engine.previous_keys[bsk.pg.K_2]:
        engine.shader = normal_shader
    if engine.keys[bsk.pg.K_3] and not engine.previous_keys[bsk.pg.K_3]:
        engine.shader = geom_shader
    engine.update()