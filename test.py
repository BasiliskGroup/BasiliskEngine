import basilisk as bsk
import glm
import numpy as np
import os

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

node = bsk.Node()
img = bsk.Image("basilisk.png")
mtl = bsk.Material('my_mtl', color=(2, 5, 3))

shader_handler = bsk.ShaderHandler(scene)

while engine.running:
    engine.update()