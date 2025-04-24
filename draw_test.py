import basilisk as bsk
import pygame as pg

engine = bsk.Engine(grab_mouse=False, max_fps=60, title=None)

brick = bsk.Image('tests/brick.png')

# Create an image from pygame surface
surf = pg.Surface((100, 100))
surf.fill((0, 255, 255))
pg.draw.rect(surf, (255, 0, 0), (10, 10, 60, 60))
pg_img = bsk.Image(surf, flip_y=True)

while engine.running:

    if engine.event_resize: print('ree')

    bsk.draw.rect(engine, (0, 0, 255), (400, 550, 200, 200))
    bsk.draw.rect(engine, 100, (0, 0, 50, 50))
    bsk.draw.line(engine, (255, 255, 255), (100, 50), (600, 300))
    bsk.draw.circle(engine, (255, 255, 0), (200, 600), 100)
    bsk.draw.blit(engine, brick, (100, 200, 200, 200), alpha=0.7)
    bsk.draw.blit(engine, pg_img, (500, 300, 150, 150))
    bsk.draw.text(engine, "test", (400, 400), 3)

    engine.update()