import glm
from .helper import get_support_point
from...nodes.node import Node


# TODO change these to structs when converting to C++
face_type     = list[tuple[float, glm.vec3, int, int, int]] # distance, normal, index 1, index 2, index 3
polytope_type = list[tuple[glm.vec3, glm.vec3, glm.vec3]] # polytope vertex, node1 vertex, node2 vertex

def get_epa_from_gjk(node1: Node, node2: Node, polytope: polytope_type, epsilon: float=1e-7) -> tuple: # TODO determine the return type of get_epa_from_gjk
    """
    Determines the peneration vector from a collision using EPA. The returned face normal is normalized but the rest are not guarunteed to be. 
    """
    # orient faces to point normals counter clockwise
    faces: face_type = []
    for face in [(0, 1, 2), (0, 1, 3), (0, 2, 3), (1, 2, 3)]:
        ...
    
    # develope the polytope until the nearest real face has been found, within epsilon
    while True:
        new_point = get_support_point(node1, node2, faces[0][1])
        if new_point in polytope or glm.length(new_point[0]) - float[0][0] < epsilon: 
            # TODO add sqrt to face distance and normalize normal
            return faces[0], polytope
        faces, polytope = insert_point(polytope, new_point)

def insert_point(polytope: polytope_type, faces: face_type, point: glm.vec3, epsilon: float=1e-7) -> tuple[face_type, polytope_type]:
    """
    Inserts a point into the polytope sorting by distance from the origin
    """ 
    # determine which faces are facing the new point
    support_index = len(polytope) - 1
    visible_faces = [
        face for face in enumerate(faces)
        if glm.dot(face[1], polytope[support_index][0]) >= epsilon and # if the normal of a face is pointing towards the added point
           glm.dot(face[1], polytope[support_index][0] - (polytope[face[2]] + polytope[face[3]] + polytope[face[4]]) / 3) >= epsilon # TODO check if this ever occurs
    ]
    
    # generate new edges
    edges = []
    for face in visible_faces:
        for p1, p2 in get_face_edges(face):
            if (p2, p1) in edges: edges.remove((p2, p1)) # edges can only be shared by two faces, running opposite to each other. 
            else: edges.append((p1, p2))
    
    # remove visible faces
    for face in sorted(visible_faces, reverse = True): del face
    
    # add new faces
    new_face_indices = [orient_face(polytope, (edge[0], edge[1], support_index)) for edge in edges] # edge[0], edge[1] is already ccw
    for indices in new_face_indices:
        distance = ... # TODO face distance is length squared to reduce calculations
        normal   = ... # closest face normal will be normalized once returned to avoid square roots and division
        new_face = (distance, normal, *indices)
        
        # insert faces into priority queue based on distance from origin
        for i, face in enumerate(faces):
            if face[0] < distance: continue
            faces.insert(i, new_face)
            break
        else: faces.append(new_face)
    
    return faces, polytope

def orient_face(polytope: polytope_type, indices: tuple[int, int, int]) -> tuple[int, int, int]: # TODO finish this
    """
    Orients the face indices to have a counter clockwise normal
    """
    return indices
    
def get_face_edges(face: tuple[float, glm.vec3, int, int, int]) -> list[tuple[int, int]]:
    """
    Permutes a tuple of three unique numbers (a, b, c) into 3 pairs (x, y), preserving order
    """
    return [(face[2], face[3]), (face[3], face[4]), (face[4], face[2])]

