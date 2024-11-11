import glm
from math import sin, cos

# getting support points
def get_support_point(points1:list, points2:list, direction_vector:glm.vec3) -> glm.vec3:
    """gets next point on a simplex"""
    point1, point2 = get_furthest_point(points1, direction_vector), get_furthest_point(points2, -direction_vector) # second vector is negative
    return (point1 - point2, point1, point2)

def get_furthest_point(points:list, direction_vector:glm.vec3) -> glm.vec3: # may need to be normalized
    """finds furthest point in given direction"""
    return max(points, key=lambda point: glm.dot(point, direction_vector))

# simple vector math
def get_average_point(polytope:list) -> glm.vec3:
    """returns the average of a convex polytope"""
    total_point = glm.vec3(0, 0, 0)
    for vector in polytope: total_point += vector[0]
    return total_point / len(polytope)

def triple_product(vector1, vector2, vector3) -> glm.vec3:
    """computes (1 x 2) x 3"""
    return glm.cross(glm.cross(vector1, vector2), vector3)

# line functions
def is_ccw_turn(a:glm.vec2, b:glm.vec2, c:glm.vec2) -> bool:
    """determines if the series of points results in a left hand turn"""
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x) > 0 # TODO check formula

# matrix math
def get_model_matrix(position, scale, rotation) -> glm.mat4x4:
    """gets projection matrix from object data"""
    # create blank matrix
    model_matrix = glm.mat4x4()
    # translate, rotate, and scale
    model_matrix = glm.translate(model_matrix, position) # translation
    model_matrix = glm.rotate(model_matrix, rotation.x, glm.vec3(-1, 0, 0)) # x rotation
    model_matrix = glm.rotate(model_matrix, rotation.y, glm.vec3(0, -1, 0)) # y rotation
    model_matrix = glm.rotate(model_matrix, rotation.z, glm.vec3(0, 0, -1)) # z rotation
    model_matrix = glm.scale(model_matrix, scale) # scale
    return model_matrix

def get_rotation_matrix(rotation) -> glm.mat3x3:
    return glm.mat3x3(
        cos(rotation[2]) * cos(rotation[1]), cos(rotation[2]) * sin(rotation[1]) * sin(rotation[0]) - sin(rotation[2]) * cos(rotation[0]), cos(rotation[2]) * sin(rotation[1]) * cos(rotation[0]) + sin(rotation[2]) * sin(rotation[0]),
        sin(rotation[2]) * cos(rotation[1]), sin(rotation[2]) * sin(rotation[1]) * sin(rotation[0]) + cos(rotation[2]) * cos(rotation[0]), sin(rotation[2]) * sin(rotation[1]) * cos(rotation[0]) - cos(rotation[2]) * sin(rotation[0]),
        -sin(rotation[1])            , cos(rotation[1]) * sin(rotation[0])                                       , cos(rotation[1]) * cos(rotation[0])                                       ,
    )

# collision formulas  
def get_aabb_collision(top_right1, bottom_left1, top_right2, bottom_left2, epsilon:float=1) -> bool:
    return all(bottom_left1[i] <= top_right2[i] + epsilon and epsilon + top_right1[i] >= bottom_left2[i] for i in range(3))

def get_aabb_line_collision(top_right:glm.vec3, bottom_left:glm.vec3, point:glm.vec3, vec:glm.vec3) -> bool:
    tmin, tmax = -1e10, 1e10
    for i in range(3):
        if vec[i] != 0:
            tlow   = (bottom_left[i] - point[i]) / vec[i]
            thigh  = (top_right[i]   - point[i]) / vec[i]
            tentry = min(tlow, thigh)
            texit  = max(tlow, thigh)
            tmin   = max(tmin, tentry)
            tmax   = min(tmax, texit)
        elif point[i] < bottom_left[i] or point[i] > top_right: return False
    return tmin <= tmax and tmax >= 0 and tmin <= 1
        
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
    
    if u < 0 or u > 1: return None
    
    s_cross = glm.cross(s, edge1)
    v = glm.dot(vec, s_cross) * inv_det
    
    if v < 0 or u + v > 1: return None
    
    t = glm.dot(edge2, s_cross) * inv_det
    if t > epsilon: return point + vec * t
    return None