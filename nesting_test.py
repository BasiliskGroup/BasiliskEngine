import basilisk as bsk

engine = bsk.Engine()
scene  = bsk.Scene()
engine.scene = scene

scene.add_node()

while engine.running:
    engine.update()
    
