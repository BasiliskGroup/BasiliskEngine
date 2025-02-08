import glm
from ...nodes.node import Node
from .dataclasses import SupportPoint

def get_support_point(node1: Node, node2: Node, dir_vec: glm.vec3) -> SupportPoint:
    """
    Outputs the best support point to be added to the polytop based on the direction vector.
    """
    vertex1, index1 = get_furthest_point(node1, dir_vec)
    vertex2, index2 = get_furthest_point(node2, -dir_vec)
    return SupportPoint(vertex1 - vertex2, index1, vertex1, index2, vertex2)
    
def get_furthest_point(node: Node, dir_vec: glm.vec3) -> glm.vec3:
    """
    Determines the furthest point in a given direction
    """
    # determine furthest point by using untransformed mesh
    node_dir_vec = node.rotation.data * dir_vec # rotate the world space vector to node space
    index = node.collider.mesh.get_best_dot(node_dir_vec)
    vertex = node.collider.mesh.points[index]
    vertex = node.model_matrix * glm.vec4(vertex, 1.0)
    
    # transform point to world space
    return glm.vec3(vertex), index

def is_ccw_turn(a:glm.vec2, b:glm.vec2, c:glm.vec2) -> bool:
    """
    Determines if the series of points results in a left hand turn
    """
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x) > 0 # TODO check formula