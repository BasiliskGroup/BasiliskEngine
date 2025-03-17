import glm

def triple_product(vector1, vector2, vector3) -> glm.vec3:
    """
    Computes (1 x 2) x 3
    """
    return glm.cross(glm.cross(vector1, vector2), vector3)

def relative_transforms(parent, child) -> tuple[glm.vec3, glm.vec3, glm.quat]:
    """
    Calculates the relative transforms for position, scale, and rotation for the parent and child. parent and child are Nodes
    """
    relative = glm.inverse(parent.model_matrix) * child.model_matrix
    position = glm.vec3(relative[3])
    scale = glm.vec3([glm.length(relative[i]) for i in range(3)])
    rotation = child.rotation.data * glm.inverse(parent.rotation.data)
    
    return position, scale, rotation