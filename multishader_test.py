import basilisk as bsk

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

default_shader = bsk.Shader(engine)
normal_shader  = bsk.Shader(engine, vert='basilisk/shaders/normal.vert', frag='basilisk/shaders/normal.frag')
geom_shader  = bsk.Shader(engine, vert='basilisk/shaders/geometry.vert', frag='basilisk/shaders/geometry.frag')

scene.add_node(position=(-4, 0, 0), shader=normal_shader)
scene.add_node(shader=default_shader)
scene.add_node(position=(4, 0, 0), shader=geom_shader)
scene.add_node(position=(0, 4, 0))

while engine.running:

    if engine.keys[bsk.pg.K_1]:
        engine.shader = default_shader
    if engine.keys[bsk.pg.K_2]:
        engine.shader = normal_shader
    if engine.keys[bsk.pg.K_3]:
        engine.shader = geom_shader

    engine.update()