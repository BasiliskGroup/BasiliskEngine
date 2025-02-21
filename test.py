import glm
import random

pitch = random.uniform(-1, 1)
yaw = random.uniform(-1, 1)
roll = random.uniform(-1, 1)
glm.quat()
rotation = glm.quat((pitch, yaw, roll))
print(yaw, pitch, roll)
print(glm.yaw(rotation), glm.pitch(rotation), glm.roll(rotation))