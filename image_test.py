import basilisk as bsk
import pygame as pg

engine = bsk.Engine()
scene = bsk.Scene(engine)

# Load image from file
img_1 = bsk.Image('tests/floor_albedo.png')

# Load image from pygame
surf = pg.Surface((400, 400))
surf.fill((100, 100, 100))
pg.draw.circle(surf, (255, 150, 150), (200, 200), 75)
img_2 = bsk.Image(surf)


mtl = bsk.Material(texture=img_1)
scene.add(bsk.Node(material=mtl))

while engine.running:

    if engine.keys[bsk.pg.K_1]: mtl.texture = img_1
    if engine.keys[bsk.pg.K_2]: mtl.texture = img_2

    scene.update()
    engine.update()