import glm

def triple_product(vector1, vector2, vector3) -> glm.vec3:
    """
    Computes (1 x 2) x 3
    """
    return glm.cross(glm.cross(vector1, vector2), vector3)