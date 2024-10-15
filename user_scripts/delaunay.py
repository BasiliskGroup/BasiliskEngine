from random import uniform
from math import sqrt, acos, sin


class Point:
    def __init__(self, x: float, y: float) -> None:
        self.x = x
        self.y = y

    def __repr__(self) -> str:
        return f'Delaunay Point: ({self.x}, {self.y})'

class Triangle:
    def __init__(self, p1: Point, p2: Point, p3: Point) -> None:
        self.p1 = p1
        self.p2 = p2
        self.p3 = p3

        self.circumcircle = get_circumcirlce(p1, p2, p3)


class Circle:
    def __init__(self, x, y, r) -> None:
        self.x = x
        self.y = y
        self.r = r

    def contains(self, p: Point):
        """
        Returns a bool denoting whether the given point is bounded by the circle
        """
        
        distance = sqrt((self.x - p.x) ** 2 + (self.y - p.y) ** 2)
        return distance < self.r
    
    
def get_super_tirangle(vertices):
    """
    Gets a triangle that bounds every vertex given
    """
    
    # Get the center point of the given vertices
    x, y = 0, 0
    for vertex in vertices:
        x += vertex.x
        y += vertex.y
    
    x /= len(vertices)
    y /= len(vertices)

    # Get the furthest distance from the center point
    r = 0
    for vertex in vertices:
        r = max(r, ((x - vertex.x) ** 2 + (y - vertex.y) ** 2))
    r = sqrt(r) + 0.01

    # Construct a triangle bounding the circle given by the center point an radius
    side_length = r * 2 * sqrt(3)

    p1 = Point(x + side_length / 2, y - r)
    p2 = Point(x - side_length / 2, y - r)
    p3 = Point(x, y - r + (side_length * sqrt(3) / 2))

    return Triangle(p1, p2, p3)

def get_circumcirlce(a: Point, b: Point, c: Point):
    """
    Gets the circle that has points a, b and c on its edge
    """
    
    # Get the side lengths
    ab = sqrt((a.x - b.x) ** 2 + (a.y - b.y) ** 2)
    bc = sqrt((b.x - c.x) ** 2 + (b.y - c.y) ** 2)
    ac = sqrt((c.x - a.x) ** 2 + (c.y - a.y) ** 2)
    # Use to get the radius
    r = (ab * bc * ac) / sqrt((ab + bc + ac) * (ab + bc - ac) * (ab - bc + ac) * (-ab + bc + ac))

    # Get the angles of the triangle
    angle_a = acos((ab ** 2 + ac ** 2 - bc ** 2) / (2 * ab * ac))
    angle_b = acos((ab ** 2 + bc ** 2 - ac ** 2) / (2 * ab * bc))
    angle_c = acos((ac ** 2 + bc ** 2 - ab ** 2) / (2 * ac * bc))

    # The position of the circle
    center_x = (a.x * sin(2 * angle_a) + b.x * sin(2 * angle_b) + c.x * sin(2 * angle_c)) / (sin(2 * angle_a) + sin(2 * angle_b) + sin(2 * angle_c))
    center_y = (a.y * sin(2 * angle_a) + b.y * sin(2 * angle_b) + c.y * sin(2 * angle_c)) / (sin(2 * angle_a) + sin(2 * angle_b) + sin(2 * angle_c))

    return Circle(center_x, center_y, r)

def compare_edges(edge_1, edge_2):
    """
    Equates edge 1 with edge 2. Returns True if they are the same, else, false
    """
    
    if edge_1[0] == edge_2[0] and edge_1[1] == edge_2[1]: return True
    if edge_1[0] == edge_2[1] and edge_1[1] == edge_2[0]: return True
    return False

def remove_duplicate_edges(edges):
    """
    Completely removes edges that appear in the given list multiple times
    """
    
    unique_edges = set(edges)
    for i, edge in enumerate(edges):
        for j, compare_edge in enumerate(edges):
            if i == j or not compare_edges(edge, compare_edge): continue
            if not edge in unique_edges: continue
            unique_edges.remove(edge)

    return unique_edges

def add_vertex(vertex, triangles):
    """
    Adds a new vertex to the triangulation
    """
    
    # Edges that are in a triangle whose circumcircle cointains the vertex
    bad_edges = []

    # Loop through each triangle and check if the given vertex is in the triangle's circumcircle
    i = 0
    while i < len(triangles):
        triangle = triangles[i]
        if triangle.circumcircle.contains(vertex):
            # The vertex is in the circumcircle
            # Add the edges of the triangle to bad_edges
            bad_edges.append((triangle.p1, triangle.p2))
            bad_edges.append((triangle.p2, triangle.p3))
            bad_edges.append((triangle.p3, triangle.p1))

            # Remove the triangle from triangles
            triangles.pop(i)
        else:
            i += 1

    # We dont want to keep edges that are shared between multiple triangles
    # This is because we want to find the "hole" that is created by adding the vertex
    # Including the shared edges would lead to redundant triangles with intersecting lines.
    bad_edges = remove_duplicate_edges(bad_edges)

    # For every edge that is in the "hole" created by the vertex, make a new triangle with the edge and vertex
    for edge in bad_edges:
        triangles.append(Triangle(*edge, vertex))
    

def delunay_triangulation(vertices):
    """
    Creates a triangluation from a list of points
    """
    
    super_triangle = get_super_tirangle(vertices)

    triangles = [super_triangle]

    for vertex in vertices: add_vertex(vertex, triangles)

    i = 0
    while i < len(triangles):
        triangle = triangles[i]

        if triangle.p1 == super_triangle.p1: triangles.pop(i); continue
        if triangle.p2 == super_triangle.p1: triangles.pop(i); continue
        if triangle.p3 == super_triangle.p1: triangles.pop(i); continue

        if triangle.p1 == super_triangle.p2: triangles.pop(i); continue
        if triangle.p2 == super_triangle.p2: triangles.pop(i); continue
        if triangle.p3 == super_triangle.p2: triangles.pop(i); continue

        if triangle.p1 == super_triangle.p3: triangles.pop(i); continue
        if triangle.p2 == super_triangle.p3: triangles.pop(i); continue
        if triangle.p3 == super_triangle.p3: triangles.pop(i); continue

        i += 1

    return triangles


vertices = [Point(uniform(-.5, .5), uniform(-.5, .5)) for i in range(50)]
tris = delunay_triangulation(vertices)