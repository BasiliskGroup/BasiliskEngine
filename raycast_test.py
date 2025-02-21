import basilisk as bsk

engine = bsk.Engine(grab_mouse=False)
scene  = bsk.Scene(engine)
scene.camera = bsk.StaticCamera()

red = bsk.Material(color=(255, 50, 50))
meshes = [None, bsk.Mesh('tests/sphere.obj')]

for x in range(-10, 10, 3):
    for y in range(-10, 10, 3):
        scene.add(bsk.Node(position=(x, y, 0), mesh=meshes[(x + y) % 2]))

while engine.running:
    
    scene.update()

    if engine.keys[bsk.pg.K_1]:
        scene.camera = bsk.StaticCamera()
        engine.mouse.grab = False
    if engine.keys[bsk.pg.K_2]:
        scene.camera = bsk.FreeCamera()
        engine.mouse.grab = True

    if isinstance(scene.camera, bsk.FreeCamera):
        bsk.draw.circle(engine, (15, 15, 15), (400, 400), 3)

    if engine.mouse.click:
        if isinstance(scene.camera, bsk.FreeCamera): res = scene.raycast()
        else: res = scene.raycast_mouse((engine.mouse.x, engine.mouse.y))
        if res: 
            scene.particle.add(position=res.position, life=.2, scale=3, mesh=res.node.mesh, material=red)
            print(res)

    engine.update()