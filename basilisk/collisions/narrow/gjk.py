import glm
from .helper import get_support_point
from .dataclasses import SupportPoint
from ...nodes.node import Node
from ...generic.math import triple_product


def collide_gjk(node1: Node, node2: Node, iterations: int=20) -> tuple: # TODO figure out return data type
    """
    Determines if two convex polyhedra collide, returns the polytope if there is a collision.
    """
    # generate starting values
    dir_vec = node1.position.data - node2.position.data
    simplex = [get_support_point(node1, node2, dir_vec)]
    dir_vec = -simplex[0].support_point # set direction to point away from starting simplex point
    
    for _ in range(iterations):
        # gets support point and checks if its across the origin
        test_point = get_support_point(node1, node2, dir_vec)
        if glm.dot(test_point.support_point, dir_vec) < -1e-7: return False, simplex
        
        # add point and find new direction vector
        simplex.append(test_point)
        check, dir_vec, simplex = handle_simplex(simplex)
        
        if check: return True, simplex
    return False, simplex # timeout due to too many checks, usually float errors
        
def handle_simplex(simplex: list[SupportPoint]) -> tuple[bool, glm.vec3, list[tuple[glm.vec3, glm.vec3, glm.vec3]]]:
    """
    Call proper function based on number of support points
    """
    num = len(simplex) # not using match case to support Python < 3.10
    if num == 2: return handle_simplex_line(simplex)
    if num == 3: return handle_simplex_triangle(simplex)
    return handle_simplex_tetrahedron(simplex) # simplex must be 4 points 

def handle_simplex_line(simplex: list[SupportPoint]) -> tuple[bool, glm.vec3, list[tuple[glm.vec3, glm.vec3, glm.vec3]]]:
    """
    Returns the perpendicular vector to the simplex line
    """
    vec_ab = simplex[1].support_point - simplex[0].support_point
    return False, triple_product(vec_ab, -simplex[0].support_point, vec_ab), simplex
    
def handle_simplex_triangle(simplex: list[SupportPoint]) -> tuple[bool, glm.vec3, list[tuple[glm.vec3, glm.vec3, glm.vec3]]]:
    """
    Returns the normal vector of the triangoe pointed towards the origin
    """
    dir_vec = glm.cross(simplex[1].support_point - simplex[0].support_point, simplex[2].support_point - simplex[0].support_point)
    return False, -dir_vec if glm.dot(dir_vec, -simplex[0].support_point) < 0 else dir_vec, simplex
    
def handle_simplex_tetrahedron(simplex: list[SupportPoint], epsilon: float=0) -> tuple[bool, glm.vec3, list[tuple[glm.vec3, glm.vec3, glm.vec3]]]:
    """
    Perform collision check and remove support point if no collision is found
    """
    vec_da = simplex[3].support_point - simplex[0].support_point
    vec_db = simplex[3].support_point - simplex[1].support_point
    vec_dc = simplex[3].support_point - simplex[2].support_point
    vec_do = -simplex[3].support_point
    
    vectors = [(glm.cross(vec_da, vec_db), 2), (glm.cross(vec_dc, vec_da), 1), (glm.cross(vec_db, vec_dc), 0)] # TODO determine if this is the best way to do this
    for normal_vec, index in vectors:
        dot_product = glm.dot(normal_vec, vec_do)
        if dot_product > epsilon:
            simplex.pop(index)
            return False, normal_vec, simplex
    return True, None, simplex