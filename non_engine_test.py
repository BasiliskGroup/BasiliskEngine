import glm

vec = glm.vec3(1, 0, 0)
quat = glm.normalize(glm.quat(7, 4, 6, 2))
# mat = glm.transpose(glm.mat3_cast(quat))
# print(vec * quat)
# print(mat[0])

# print(vec * quat)
print(vec * glm.inverse(quat))
print(quat * vec)
# print(glm.inverse(quat) * vec)