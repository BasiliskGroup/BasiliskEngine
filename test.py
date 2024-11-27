import basilisk as bsk

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

node = bsk.Node()
img = bsk.Image(engine, "basilisk.png")
print(img)

while engine.running:
    engine.update()