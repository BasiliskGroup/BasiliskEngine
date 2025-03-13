import glm
from ..generic.vec3 import Vec3
from ..generic.quat import Quat


# transform matrices
def get_model_matrix(position: Vec3, scale: Vec3, rotation: Quat) -> glm.mat4x4:
    """
    Gets projection matrix from object data
    """
    translation_matrix  = glm.translate(glm.mat4(1.0), position.data)
    rotation_matrix     = glm.mat4_cast(rotation.data)
    scale_matrix        = glm.scale(glm.mat4(1.0), scale.data)
    model_matrix        = translation_matrix * glm.transpose(rotation_matrix) * scale_matrix
    return model_matrix

def get_scale_matrix(scale: glm.vec3) -> glm.mat3x3:
    """
    Gets the scaling matrix from a scale vector
    """
    return glm.mat3x3(
        scale.x, 0, 0,
        0, scale.y, 0,
        0, 0, scale.z
    )

# inertia tensors
def compute_inertia_moment(t:list[glm.vec3], i:int) -> float:
    return t[0][i] ** 2 + t[1][i] * t[2][i] + \
           t[1][i] ** 2 + t[0][i] * t[2][i] + \
           t[2][i] ** 2 + t[0][i] * t[1][i]
           
def compute_inertia_product(t:list[glm.vec3], i:int, j:int) -> float:
    return 2 * t[0][i] * t[0][j] + t[1][i] * t[2][j] + t[2][i] * t[1][j] + \
           2 * t[1][i] * t[1][j] + t[0][i] * t[2][j] + t[2][i] * t[0][j] + \
           2 * t[2][i] * t[2][j] + t[0][i] * t[1][j] + t[1][i] * t[0][j]