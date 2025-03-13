import basilisk as bsk
import glm

engine = bsk.Engine()
scene = bsk.Scene(engine)

node = bsk.Node()
scene.add(node)

t = 0
t_final = 3
q_final = glm.quat(0.707, 0, 0.707, 0)
v_final = glm.vec3(0, 20, 0)

while engine.running:
    
    
    
    scene.update()
    engine.update()