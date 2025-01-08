import glm
import numpy as np


# transformations
def transform_points(points: np.ndarray, model_matrix: glm.mat4x4) -> list[glm.vec3]:
    """
    Transforms the mesh points to world space
    """
    return [model_matrix * pt for pt in points]

# bvh
def get_aabb_surface_area(top_right: glm.vec3, bottom_left: glm.vec3) -> float:
    """
    Returns the surface area of the AABB
    """
    diagonal = top_right - bottom_left
    return 2 * (diagonal.x * diagonal.y + diagonal.y * diagonal.z + diagonal.x * diagonal.z)

def get_extreme_points_np(points: np.ndarray) -> tuple[glm.vec3, glm.vec3]:
    """
    Returns the top right and bottom left points of the aabb encapsulating the points
    """
    top_right   = glm.vec3(-1e10)
    bottom_left = glm.vec3(1e10)
    for pt in points:
        for i in range(3):
            if top_right[i] < pt[i]: top_right[i] = pt[i]
            if bottom_left[i] > pt[i]: bottom_left[i] = pt[i]
    return top_right, bottom_left

def get_aabb_line_collision(top_right:glm.vec3, bottom_left:glm.vec3, point:glm.vec3, vec:glm.vec3) -> bool:
    """
    Determines if a line has collided with an aabb
    """
    tmin, tmax = -1e10, 1e10
    for i in range(3):
        if vec[i] != 0:
            inv_dir = 1.0 / vec[i]
            t1 = (bottom_left[i] - point[i]) * inv_dir
            t2 = (top_right[i]   - point[i]) * inv_dir
            t1, t2 = min(t1, t2), max(t1, t2)
            tmin   = max(tmin, t1)
            tmax   = min(tmax, t2)
            if tmin > tmax: return False
        elif point[i] < bottom_left[i] or point[i] > top_right[i]: return False
    return tmax >= 0 and tmin <= 1

def moller_trumbore(point:glm.vec3, vec:glm.vec3, triangle:list[glm.vec3], epsilon:float=1e-7) -> glm.vec3 | None:
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