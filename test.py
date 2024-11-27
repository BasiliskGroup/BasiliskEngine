import basilisk as bsk
import glm
import numpy as np

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

node = bsk.Node()
img = bsk.Image("basilisk.png")
mtl = bsk.Material('my_mtl', color=(2, 5, 3))

while engine.running:
    engine.update()