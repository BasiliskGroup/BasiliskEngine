"""
Node hierarchy matrix stack test
"""

import basilisk as bsk
import glm
from math import sin, cos

engine = bsk.Engine()
scene = bsk.Scene(engine)

node1 = bsk.Node(scene)
node2 = bsk.Node(node1, position=(3, 2, 0))
node3 = bsk.Node(node2, position=(3, 2, 0))

t = 0

while engine.is_running():
    engine.update()
    scene.update()

    t += engine.get_delta_time()

    # Move the base side to side
    x = sin(t) * 2
    y = 0
    z = cos(t) * 2
    node1.set_position((x, y, z))

    # Rotate the base node around the y axis
    rot = glm.rotate(glm.quat(1, 0, 0, 0), t, (0, 1, 0))
    node1.set_rotation((rot.w, rot.x, rot.y, rot.z))

    # Rotate the second node around the X axis
    rot = glm.rotate(glm.quat(1, 0, 0, 0), t, (1, 0, 0))
    node2.set_rotation((rot.w, rot.x, rot.y, rot.z))

    # Rotate the third node around the Z axis
    rot = glm.rotate(glm.quat(1, 0, 0, 0), t, (0, 0, 1))
    node3.set_rotation((rot.w, rot.x, rot.y, rot.z))

    # Scale the base node up and down
    s = cos(t) / 2 + 1
    node1.set_scale((s, s, s))

    scene.render()
    engine.render()