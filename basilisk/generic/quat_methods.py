import glm


def rotate_vec_by_quat(vec: glm.vec3, quat: glm.quat) -> glm.vec3:
    """
    Rotates a vector by a quaternion. Probably just dont use this, just a reminder of how glm works with quaternions
    """
    return vec * quat