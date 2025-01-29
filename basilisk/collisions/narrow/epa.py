import glm
from .helper import get_support_point
from .dataclasses import SupportPoint
from...nodes.node import Node


# TODO change these to structs when converting to C++
face_type     = list[tuple[float, glm.vec3, glm.vec3, int, int, int]] # distance, normal, center, index 1, index 2, index 3
polytope_type = list[SupportPoint] # polytope vertex, node1 vertex, node2 vertex

def get_epa_from_gjk(node1: Node, node2: Node, polytope: polytope_type, epsilon: float=0) -> tuple[face_type, polytope_type]: # TODO determine the return type of get_epa_from_gjk and if epsilon is good value
    """
    Determines the peneration vector from a collision using EPA. The returned face normal is normalized but the rest are not guarunteed to be. 
    """
    # orient faces to point normals counter clockwise
    faces: face_type = []
    for indices in [(0, 1, 2), (0, 1, 3), (0, 2, 3), (1, 2, 3)]: faces = insert_face(polytope, faces, indices)
    
    # develope the polytope until the nearest real face has been found, within epsilon
    while True:
        new_point = get_support_point(node1, node2, faces[0][1])
        if new_point in polytope or glm.length(new_point.support_point) - faces[0][0] < epsilon: return faces[0], polytope
        faces, polytope = insert_point(polytope, faces, new_point)

def insert_point(polytope: polytope_type, faces: face_type, point: glm.vec3, epsilon: float=0) -> tuple[face_type, polytope_type]:
    """
    Inserts a point into the polytope sorting by distance from the origin
    """ 
    # determine which faces are facing the new point
    polytope.append(point)
    support_index = len(polytope) - 1
    visible_faces = [
        face for face in faces
        if glm.dot(face[1], polytope[support_index].support_point) >= epsilon and # if the normal of a face is pointing towards the added point
           glm.dot(face[1], polytope[support_index].support_point - face[2]) >= epsilon # TODO check if this ever occurs
    ]
    
    # generate new edges
    edges = []
    for face in visible_faces:
        for p1, p2 in get_face_edges(face):
            if (p2, p1) in edges: edges.remove((p2, p1)) # edges can only be shared by two faces, running opposite to each other. 
            elif (p1, p2) in edges: # TODO remove this
                edges.remove((p1, p2))
                # print('not reversed')
            else: edges.append((p1, p2))
    
    # remove visible faces
    for face in sorted(visible_faces, reverse = True): faces.remove(face)
    
    # add new faces
    new_face_indices = [orient_face(polytope, (edge[0], edge[1], support_index)) for edge in edges] # edge[0], edge[1] is already ccw
    for indices in new_face_indices: faces = insert_face(polytope, faces, indices)
    
    return faces, polytope

def insert_face(polytope: polytope_type, faces: face_type, indices: tuple[int, int, int]) -> face_type:
    """
    Inserts a face into the face priority queue based on the indices given in the polytope
    """
    center   = (polytope[indices[0]].support_point + polytope[indices[1]].support_point + polytope[indices[2]].support_point) / 3
    normal   = glm.cross(polytope[indices[1]].support_point - polytope[indices[0]].support_point, polytope[indices[2]].support_point - polytope[indices[0]].support_point) # closest face normal will be normalized once returned to avoid square roots and division
    if glm.dot(center, normal) < 0: 
        normal *= -1
        indices = (indices[2], indices[1], indices[0])

    # TODO solve cases where face may contain origin
    normal = glm.normalize(normal)
    distance = abs(glm.dot(polytope[indices[0]].support_point, normal))
    new_face = (distance, normal, center, *indices)
    
    # insert faces into priority queue based on distance from origin
    for i, face in enumerate(faces):
        if face[0] < distance: continue
        faces.insert(i, new_face)
        break
    else: faces.append(new_face)
    
    return faces

def orient_face(polytope: polytope_type, indices: tuple[int, int, int]) -> tuple[int, int, int]:
    """
    Orients the face indices to have a counter clockwise normal
    """
    if glm.dot(glm.cross(polytope[indices[1]].support_point, polytope[indices[2]].support_point), polytope[indices[0]].support_point) < 0: return (indices[2], indices[1], indices[0])
    return indices
    
def get_face_edges(face: tuple[float, glm.vec3, glm.vec3, int, int, int]) -> list[tuple[int, int]]:
    """
    Permutes a tuple of three unique numbers (a, b, c) into 3 pairs (x, y), preserving order
    """
    return [(face[3], face[4]), (face[4], face[5]), (face[5], face[3])]