import glm
from ..generic.vec3 import Vec3
from ..generic.quat import Quat


def collide_aabb_aabb(top_right1: glm.vec3, bottom_left1: glm.vec3, top_right2: glm.vec3, bottom_left2: glm.vec3, epsilon:float=1e-7) -> bool:
    """
    Determines if two aabbs are colliding
    """
    return all(bottom_left1[i] <= top_right2[i] + epsilon and epsilon + top_right1[i] >= bottom_left2[i] for i in range(3))

def collide_aabb_line(top_right: glm.vec3, bottom_left: glm.vec3, position: glm.vec3, forward: glm.vec3) -> bool: # TODO check algorithm
    """
    Determines if an infinite line intersects with an AABB
    """
    # based on Smit's algorithm
    inv_dir = glm.vec3([1 / i if i != 0 else float('inf') for i in forward])

    t1 = (bottom_left - position) * inv_dir
    t2 = (top_right - position) * inv_dir

    t_min = glm.min(t1, t2)  # entry point along each axis
    t_max = glm.max(t1, t2)  # exit point along each axis

    t_entry = max(t_min.x, t_min.y, t_min.z)  # furthest entry time
    t_exit = min(t_max.x, t_max.y, t_max.z)  # earliest exit time

    return t_entry <= t_exit and t_exit >= 0

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

def get_sat_axes(rotation1: Quat, rotation2: Quat) -> list[glm.vec3]:
    """
    Gets the axes for SAT from obb rotation matrices
    """
    axes = []
    axes.extend(glm.transpose(glm.mat3_cast(rotation1.data)))
    axes.extend(glm.transpose(glm.mat3_cast(rotation2.data)))
    
    # crossed roots
    for i in range(0, 3):
        for j in range(3, 6):
            cross = glm.cross(axes[i], axes[j])
            if glm.length2(cross) < 1e-6: continue
            axes.append(glm.normalize(cross))
            
    return axes