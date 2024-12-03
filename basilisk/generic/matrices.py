import glm

# transform matrices
def get_model_matrix(position: glm.vec3, scale: glm.vec3, rotation: glm.quat) -> glm.mat4x4:
    """
    Gets projection matrix from object data
    """
    # create blank matrix
    model_matrix = glm.mat4x4()
    # translate, rotate, and scale
    model_matrix = glm.translate(model_matrix, position) # translation
    model_matrix = glm.rotate(model_matrix, rotation.x, glm.vec3(-1, 0, 0)) # x rotation
    model_matrix = glm.rotate(model_matrix, rotation.y, glm.vec3(0, -1, 0)) # y rotation
    model_matrix = glm.rotate(model_matrix, rotation.z, glm.vec3(0, 0, -1)) # z rotation
    model_matrix = glm.scale(model_matrix, scale) # scale
    
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

def get_rotation_matrix(q: glm.quat) -> glm.mat3x3:
    """
    Gets the rotation matrix representation of a quaternion
    """
    return glm.mat3x3(
        2 * (q[0] ** 2 + q[1] ** 2) - 1, 2 * (q[1] * q[2] - q[0] * q[3]), 2 * (q[1] * q[3] + q[0] * q[2]),
        2 * (q[1] * q[2] + q[0] * q[3]), 2 * (q[0] ** 2 + q[2] ** 2) - 1, 2 * (q[2] * q[3] - q[0] * q[1]),
        2 * (q[1] * q[3] - q[0] * q[2]), 2 * (q[2] * q[3] + q[0] * q[1]), 2 * (q[0] ** 2 + q[3] ** 2) - 1
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