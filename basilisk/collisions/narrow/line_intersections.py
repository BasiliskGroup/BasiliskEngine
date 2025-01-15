import glm
from .helper import is_ccw_turn

# intersectionn algorithms for lines
def line_line_intersect(points1:list[glm.vec2], points2:list[glm.vec2]) -> list[glm.vec2]:
    """gets the intersection of 2 2d lines. if the lines are parallel, returns the second line"""
    # orders points from smallest x to greatest x
    points1 = sorted(points1, key=lambda p: (p.x, p.y))
    points2 = sorted(points2, key=lambda p: (p.x, p.y))
    vec1, vec2 = points1[1] - points1[0], points2[1] - points2[0]
    
    # if vectors have the same slope return the smallest line
    if have_same_slope(vec1, vec2): return sorted(points1 + points2, key=lambda p: (p.x, p.y))[1:3]
    
    # line - line intersection
    det = vec1.x * vec2.y - vec1.y * vec2.x
    if det == 0: return []
    t = (points2[0].x - points1[0].x) * vec2.y - (points2[0].y - points1[0].y) * vec2.x
    t /= det
    return [points1[0] + t * vec1]
    
def have_same_slope(vec1:glm.vec2, vec2:glm.vec2, epsilon:float=1e-5) -> bool:
    """determines if two vectors moving in the positive x direction have the same slope"""
    return abs(vec1.y * vec2.x - vec2.y * vec1.x) < epsilon

def line_poly_intersect(line:list[glm.vec2], polygon:list[glm.vec2]) -> list[glm.vec2]: #TODO Reseach into faster algorithm < O(2n)
    """computes which parts of the line clip with the polygon"""
    # calculate the center of the polygon
    assert len(polygon) > 2, 'polygon is does not contain engough points'
    center = glm.vec2(0,0)
    for point in polygon: center += point
    center /= len(polygon)
    orig_line = line[:]
    # determine which points are in or out of the polygon
    exterior_points = []
    for i in range(len(polygon)): # nearest even number below n
        for line_point in line[:]:
            if not is_ccw_turn(polygon[i], polygon[(i + 1) % len(polygon)], line_point): # if point is on the outside
                exterior_points.append((polygon[i], polygon[(i + 1) % len(polygon)], line_point)) # polypoint1, polypoint2, linepoint
                line.remove(line_point) # removes line point if it is confirmed to be outside
                
    # determine what to with line based on number of points found outside
    if len(exterior_points) == 0:
        return line
    if len(exterior_points) == 1:
        return line_line_intersect(line + [exterior_points[0][2]], exterior_points[0][0:2]) + [line[0]] # [intersecting point, exterior point]
    if len(exterior_points) == 2: # line must intersect with two edges
        points = []
        for i in range(len(polygon)):
            intersection = line_line_intersect(orig_line, [polygon[i], polygon[(i + 1) % len(polygon)]])
            if len(intersection) > 0: points += intersection
            if len(points) > 1: break # exit if two intersections have been found
        else: return [] # fallback if 0 or one intersections found
        return points
    
def closest_two_lines(p1: glm.vec3, q1: glm.vec3, p2: glm.vec3, q2: glm.vec3, epsilon: float=1e-7) -> tuple[glm.vec3, glm.vec3]:
    """
    Determines the closest point on each line segment to the other line segment. 
    """
    # create direction vector
    d1 = q1 - p1
    d2 = q2 - p2
    r = p1 - p2
    
    # get lengths of line segments
    a = glm.dot(d1, d1)
    e = glm.dot(d2, d2)
    f = glm.dot(d2, r)
    
    # check if either or both line segment degenerate into points
    if a <= epsilon and e <= epsilon:
        # both segments degenerate
        return p1, p2
    
    if a <= epsilon:
        s = 0
        t = glm.clamp(f / e, 0, 1)
    else: 
        c = glm.dot(d1, r)
        if e <= epsilon:
            # the second line degenerates to a point
            t = 0
            s = glm.clamp(-c / a, 0, 1)
        else:
            # if neither of them degenerate to a point
            b = glm.dot(d1, d2)
            denom = a * e - b ** 2 # this will always be non-negative
            
            # if segments are not parallel, compute closest point from l1 to l2
            s = glm.clamp((b * f - c * e) / denom, 0, 1) if denom else 0
            
            # compute closest point from l2 on s1(s)
            t = (b * s + f) / e
            
            # if t is not in [0, 1], clamp and recompute s
            if t < 0:
                t = 0
                s = glm.clamp(-c / a, 0, 1)
            elif t > 1:
                t = 1
                s = glm.clamp((b - c) / a, 0, 1)
                
    c1 = p1 + d1 * s
    c2 = p2 + d2 * t
    return c1, c2
                
        