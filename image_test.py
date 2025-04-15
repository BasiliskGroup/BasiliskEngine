import basilisk as bsk
import pygame as pg

engine = bsk.Engine()
scene = bsk.Scene(engine)

# Load image from file
img_1 = bsk.Image('tests/floor_albedo.png')

# Load image from pygame
surf = pg.Surface((400, 400))
surf.fill((100, 100, 100))
pg.draw.circle(surf, (255, 150, 150), (100, 100), 75)

img_2 = bsk.Image(surf, flip_x=False, flip_y=False)
img_3 = bsk.Image(surf, flip_x=True, flip_y=False)
img_4 = bsk.Image(surf, flip_x=False, flip_y=True)
img_5 = bsk.Image(surf, flip_x=True, flip_y=True)



mtl = bsk.Material(texture=img_1)
scene.add(bsk.Node(material=mtl))

while engine.running:

    if engine.keys[bsk.pg.K_1]: mtl.texture = img_1
    if engine.keys[bsk.pg.K_2]: mtl.texture = img_2
    if engine.keys[bsk.pg.K_3]: mtl.texture = img_3
    if engine.keys[bsk.pg.K_4]: mtl.texture = img_4
    if engine.keys[bsk.pg.K_5]: mtl.texture = img_5

    scene.update()
    engine.update()