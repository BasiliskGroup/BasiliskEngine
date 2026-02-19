import basilisk as bsk
import random

engine = bsk.Engine(800, 800, "Basilisk")
scene = bsk.Scene(engine)

img = bsk.Image("textures/container.jpg")
image = bsk.Image([1.0, 0.0, 0.0, 1.0], 1, 1)
mtl = bsk.Material((1, 1, 1), image, image)
mesh = bsk.Mesh("models/cube.obj", False, False)


# Make node
node = bsk.Node(scene, mesh, mtl, (0, 0, 0), (0, 0, 0, 1), (1, 1, 1))
scene.add(node)

# Add ambient light
light1 = bsk.AmbientLight((1, 1, 1), 0.2)
scene.add(light1)

# Add directional light
light2 = bsk.DirectionalLight((1, 1, 1), 1.0, (0, -1, 0.2))
scene.add(light2)

# Add point light
light3 = bsk.PointLight((1, 1, 1), 1.0, (0, 0, 0), 8.0)
scene.add(light3)

# Add skybox
cubemap = bsk.Cubemap(["textures/skybox/right.jpg", "textures/skybox/left.jpg", "textures/skybox/top.jpg", "textures/skybox/bottom.jpg", "textures/skybox/front.jpg", "textures/skybox/back.jpg"])
skybox = bsk.Skybox(cubemap)
scene.set_skybox(skybox)

while engine.is_running():
    engine.update()
    scene.update()
    scene.render()
    engine.render()