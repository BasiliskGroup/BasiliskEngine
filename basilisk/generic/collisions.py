import glm

def collide_aabb_aabb(top_right1: glm.vec3, bottom_left1: glm.vec3, top_right2: glm.vec3, bottom_left2: glm.vec3, epsilon:float=1) -> bool:
    return all(bottom_left1[i] <= top_right2[i] + epsilon and epsilon + top_right1[i] >= bottom_left2[i] for i in range(3))