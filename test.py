import glm

rotation = glm.quat(0.707, 0.707, 0, 0)
dt = 1
rotational_velocity = glm.normalize(glm.vec3(0, glm.pi(), 0))

dq = rotation * rotational_velocity

print(rotational_velocity)
print(dq)
print(dq * rotation)