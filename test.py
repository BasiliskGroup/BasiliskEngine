import basilisk as bsk
import glm

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

node = bsk.Node()
img = bsk.Image("basilisk.png")
clr = glm.vec3(255, 2, 25)
mtl = bsk.Material('hello', color=clr, texture=img)
print(mtl.texture)

while engine.running:
    engine.update()