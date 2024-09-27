import glm

# main epa handler
def get_epa_from_gjk(points1:list, points2:list, polytope:list) -> tuple:
    """gets normal and distance from expanding polytope expansion"""
    # each list indexes the points of a face, always the same for converted simplexes
    faces = [(0, 1, 2), (0, 1, 3), (0, 2, 3), (1, 2, 3)]
    
    # calculate face normals
    avg_pt = get_average_point(polytope)
    normals = [calculate_polytope_normal(face, polytope, avg_pt) for face in faces]
    # minimum collision response variables
    while True: 
        nearest_normal, nearest_distance, nearest_face = get_nearest(polytope, faces, normals)
        new_point = get_support_point(points1, points2, nearest_normal)
        # tests new point distance
        if new_point in polytope or glm.length(new_point[0]) - nearest_distance < 0:
            # find contact points
            return nearest_normal, nearest_distance, polytope, nearest_face
        polytope.append(new_point) # add support point to polytope
        faces, normals = get_new_faces_and_normals(faces, normals, polytope) # find new faces on polytope
        
# polytope handling
def get_nearest(polytope:list, faces:list, normals:list) -> tuple:
    """returns the normal and distance of nearest face"""
    nearest, nearest_distance, nearest_face = None, 1e10, None
    for i, face in enumerate(faces):
        if (distance := abs(glm.dot(polytope[face[0]][0], normals[i]))) < nearest_distance: nearest, nearest_distance, nearest_face = normals[i], distance, face
    return nearest, nearest_distance, nearest_face

def get_new_faces_and_normals(faces:list, normals:list, polytope:list) -> tuple:
    """
    returns new faces and normals of polytope with added point
    polytope must contain recent support point as last index
    """
    sp_index, visible_indexes = len(polytope) - 1, []
    visible_indexes = [
        i for i, normal in enumerate(normals)
        if glm.dot(normal, polytope[sp_index][0]) >= 1e-5 and
           glm.dot(polytope[sp_index][0] - ((polytope[faces[i][0]][0] +
           polytope[faces[i][1]][0] + polytope[faces[i][2]][0]) / 3), normal) >= 1e-5
    ]
        
    # finds new edges
    edges = []
    for i in visible_indexes:
        for edge in get_face_edges(faces[i]):
            if edge in edges: edges.remove(edge)
            else: edges.append(edge)
            
    # remove visible faces
    for i in sorted(visible_indexes, reverse=True):
        del faces[i]
        del normals[i]
        
    # adds new faces
    new_faces = [(edge[0], edge[1], sp_index) for edge in edges]
    faces.extend(new_faces)
    
    # calculate new normals
    avg_pt = get_average_point(polytope)
    #for face in faces[len(normals):]: normals.append(calculate_polytope_normal(face, polytope, avg_pt))
    normals.extend(calculate_polytope_normal(face, polytope, avg_pt) for face in new_faces)
    return faces, normals

def get_face_edges(face:list) -> list:
    """returns the edge indexes to the polytope points"""
    return [(one, two) if (one := face[i - 1]) < (two := face[i]) else (two, one) for i in range(3)]

def calculate_polytope_normal(face:list, polytope:list, reference_center:glm.vec3) -> glm.vec3:
    """calculates the given normal from 3 points on the polytope"""
    one, two, three = polytope[face[0]][0], polytope[face[1]][0], polytope[face[2]][0]
    normal = glm.cross(one-two, one-three)
    # calculate average point
    if glm.dot((one + two + three)/3 - reference_center, normal) < 0: normal *= -1
    return glm.normalize(normal)

# getting support points
def get_support_point(points1:list, points2:list, direction_vector:glm.vec3) -> glm.vec3:
    """gets next point on a simplex"""
    point1, point2 = get_furthest_point(points1, direction_vector), get_furthest_point(points2, -direction_vector) # second vector is negative
    return (point1 - point2, point1, point2)

def get_furthest_point(points:list, direction_vector:glm.vec3) -> glm.vec3: # may need to be normalized
    """finds furthest point in given direction"""
    return max(points, key=lambda point: glm.dot(point, direction_vector))

def get_average_point(polytope:list) -> glm.vec3:
    """returns the average of a convex polytope"""
    return sum((vector[0] for vector in polytope)) / len(polytope)