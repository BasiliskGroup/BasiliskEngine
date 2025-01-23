import basilisk as bsk
import numpy as np

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

dirt_img = bsk.Image("tests/dirt.png")
grass_side_img = bsk.Image("tests/grass_block_side.png")
grass_top_img = bsk.Image("tests/grass_block_top.png")

dirt = bsk.Material(texture=dirt_img)
grass_side = bsk.Material(texture=grass_side_img)
grass_top = bsk.Material(texture=grass_top_img)

red = bsk.Material(color=(255, 0, 0))
blue = bsk.Material(color=(0, 0, 255))

quad = np.array([[0, 0, 0, 0, 0], [5, 0, 5, 1, 1], [5, 0, 0, 1, 0], 
                 [0, 0, 5, 0, 1], [5, 0, 5, 1, 1], [0, 0, 0, 0, 0]])

mesh = bsk.Mesh(quad)

dirt_mtl = [grass_side] * (6 * 6)
for i in range(6): 
    dirt_mtl[i] = grass_top
    dirt_mtl[i+6] = dirt
    dirt_mtl[i+12] = red
    dirt_mtl[i+18] = blue

scene.add_node(material=dirt_mtl)
scene.camera.position = (2, 3, 2)

while engine.running:
    engine.update()