import basilisk as bsk
import numpy as np

engine = bsk.Engine()
scene = bsk.Scene(engine)

wood_img = bsk.Image('tests/floor_albedo.png')

red = bsk.Material(color=(255, 0, 0))
green = bsk.Material(color=(0, 255, 0))
blue = bsk.Material(color=(0, 0, 255))
wood = bsk.Material(texture=wood_img)
swap = bsk.Material()

quad = np.array([[0, 0, 0, 0, 0], [5, 0, 5, 1, 1], [5, 0, 0, 1, 0], 
                 [0, 0, 5, 0, 1], [5, 0, 5, 1, 1], [0, 0, 0, 0, 0]])

mesh = bsk.Mesh(quad)

node = bsk.Node(mesh=mesh)
scene.add(node)
scene.camera.position = (2.5, 2, 10)

while engine.running:
    scene.update()

    if engine.keys[bsk.pg.K_1]:
        node.material = red
    elif engine.keys[bsk.pg.K_2]:
        node.material = wood
    elif engine.keys[bsk.pg.K_3]:
        node.material = None
    elif engine.keys[bsk.pg.K_4]:
        node.material = [red, blue]
    elif engine.keys[bsk.pg.K_5]:
        node.material = [blue, red]
    elif engine.keys[bsk.pg.K_6]:
        node.material = [green, wood]
    elif engine.keys[bsk.pg.K_7]:
        node.material = swap
    elif engine.keys[bsk.pg.K_8]:
        swap.color = (0, 0, 255)
    elif engine.keys[bsk.pg.K_9]:
        swap.texture = wood_img

    engine.update()