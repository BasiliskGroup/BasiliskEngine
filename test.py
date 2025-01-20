import glm

rotation = glm.quat(0.707, 0.707, 0, 0)
dt = 1
rotational_velocity = glm.normalize(glm.vec3(0, glm.pi(), 0))

print('rv', glm.quat(0, rotational_velocity))

dq = 0.5 * rotation * glm.quat(0, rotational_velocity)

print('dq', dq)

rotation += dt * dq

print(rotation)

rotation = glm.normalize(rotation)

print(rotation)