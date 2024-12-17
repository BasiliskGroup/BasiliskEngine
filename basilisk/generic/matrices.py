import glm


# transform matrices
def get_model_matrix(position: glm.vec3, scale: glm.vec3, rotation: glm.quat) -> glm.mat4x4:
    """
    Gets projection matrix from object data
    """
    translation_matrix  = glm.translate(glm.mat4(1.0), position)
    rotation_matrix     = glm.mat4_cast(rotation)
    scale_matrix        = glm.scale(glm.mat4(1.0), scale)
    model_matrix        = translation_matrix * glm.transpose(rotation_matrix) * scale_matrix
    return model_matrix

# mat4 getModelMatrix(vec3 pos, vec4 rot, vec3 scl) {
#     mat4 translation = mat4(
#         1    , 0    , 0    , 0,
#         0    , 1    , 0    , 0,
#         0    , 0    , 1    , 0,
#         pos.x, pos.y, pos.z, 1
#     );
#     mat4 rotation = mat4(
#         1 - 2 * (rot.z * rot.z + rot.w * rot.w), 2 * (rot.y * rot.z - rot.w * rot.x), 2 * (rot.y * rot.w + rot.z * rot.x), 0,
#         2 * (rot.y * rot.z + rot.w * rot.x), 1 - 2 * (rot.y * rot.y + rot.w * rot.w), 2 * (rot.z * rot.w - rot.y * rot.x), 0,
#         2 * (rot.y * rot.w - rot.z * rot.x), 2 * (rot.z * rot.w + rot.y * rot.x), 1 - 2 * (rot.y * rot.y + rot.z * rot.z), 0,
#         0, 0, 0, 1
#     );
#     mat4 scale = mat4(
#         scl.x, 0    , 0    , 0,
#         0    , scl.y, 0    , 0,
#         0    , 0    , scl.z, 0,
#         0    , 0    , 0    , 1
#     );
#     return translation * rotation * scale;
# }

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