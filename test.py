import basilisk as bsk
import glm
import numpy as np
import os

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

mesh = bsk.Mesh('cube.obj')
img = bsk.Image("basilisk.png")
mtl = bsk.Material('my_mtl', color=(2, 5, 3), texture=img, normal=img)
node = bsk.Node(mesh=mesh, material=mtl)

while engine.running:
    engine.update()