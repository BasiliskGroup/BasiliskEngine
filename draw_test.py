import basilisk as bsk

engine = bsk.Engine(grab_mouse=False)
scene = bsk.Scene()
engine.scene = scene

brick = bsk.Image('tests/brick.png')

while engine.running:

    bsk.draw.rect(engine, (0, 0, 255), (400, 550, 200, 200))
    bsk.draw.line(engine, (255, 255, 255), (100, 200), (600, 300))
    bsk.draw.circle(engine, (255, 255, 0), (200, 600), 100)
    bsk.draw.blit(engine, brick, (500, 300, 200, 200))

    engine.update()