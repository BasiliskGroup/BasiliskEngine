import glm


def collide_aabb_aabb(top_right1: glm.vec3, bottom_left1: glm.vec3, top_right2: glm.vec3, bottom_left2: glm.vec3, epsilon:float=1e-7) -> bool:
    """
    Determines if two aabbs are colliding
    """
    return all(bottom_left1[i] <= top_right2[i] + epsilon and epsilon + top_right1[i] >= bottom_left2[i] for i in range(3))

def collide_aabb_line(top_right: glm.vec3, bottom_left: glm.vec3, position: glm.vec3, forward: glm.vec3) -> bool: # TODO check algorithm
    """
    Determines if an infinite line intersects with an AABB
    """
    tmin, tmax = -1e10, 1e10
    for i in range(3):
        if forward[i]: # if forward[i] is not 0 to avoid division errors
            
            deno = 1 / forward[i]
            tlow   = (bottom_left[i] - position[i]) * deno
            thigh  = (top_right[i]   - position[i]) * deno
            if deno < 0: tlow, thigh = thigh, tlow
            tmin   = max(tmin, tlow)
            tmax   = min(tmax, thigh)
            if tmax <= tmin: return False
            
        elif position[i] + 1e-7 < bottom_left[i] or position[i] > top_right[i] + 1e-7: return False

    return True

def moller_trumbore(point:glm.vec3, vec:glm.vec3, triangle:list[glm.vec3], epsilon:float=1e-7) -> glm.vec3:
    """
    Determines where a line intersects with a triangle and where that intersection occurred
    """
    edge1, edge2 = triangle[1] - triangle[0], triangle[2] - triangle[0]
    ray_cross = glm.cross(vec, edge2)
    det = glm.dot(edge1, ray_cross)
    
    # if the ray is parallel to the triangle
    if abs(det) < epsilon: return None
    
    inv_det = 1 / det
    s = point - triangle[0]
    u = glm.dot(s, ray_cross) * inv_det
    
    if (u < 0 and abs(u) > epsilon) or (u > 1 and abs(u - 1) > epsilon): return None
    
    s_cross = glm.cross(s, edge1)
    v = glm.dot(vec, s_cross) * inv_det
    
    if (v < 0 and abs(v) > epsilon) or (u + v > 1 and abs(u + v - 1) > epsilon): return None
    
    t = glm.dot(edge2, s_cross) * inv_det
    if t > epsilon: return point + vec * t
    return None

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
            if glm.length2(cross) < 1e-6: continue
            axes.append(glm.normalize(cross))
            
    return axes