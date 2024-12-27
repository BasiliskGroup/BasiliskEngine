import glm

def collide_aabb_aabb(top_right1: glm.vec3, bottom_left1: glm.vec3, top_right2: glm.vec3, bottom_left2: glm.vec3, epsilon:float=1) -> bool:
    """
    Determines if two aabbs are colliding
    """
    return all(bottom_left1[i] <= top_right2[i] + epsilon and epsilon + top_right1[i] >= bottom_left2[i] for i in range(3))

def get_sat_axes(rotation1: glm.quat, rotation2: glm.quat) -> list[glm.vec3]:
    """
    Gets the axes for SAT from obb rotation matrices
    """
    axes = []
    axes.extend(glm.transpose(glm.mat3_cast(rotation1)))
    axes.extend(glm.transpose(glm.mat3_cast(rotation2)))
    
    # crossed roots
    for i in range(0, 3):
        for j in range(3, 6):
            cross = glm.cross(axes[i], axes[j])
            if glm.length2(cross) > 1e-6: continue
            axes.append(glm.normalize(cross))
            
    return axes