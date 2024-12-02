import glm

# transform matrices
def get_model_matrix(position: glm.vec3, scale: glm.vec3, rotation: glm.quat) -> glm.mat4x4:
    """gets projection matrix from object data"""
    # create blank matrix
    model_matrix = glm.mat4x4()
    # translate, rotate, and scale
    model_matrix = glm.translate(model_matrix, position) # translation
    model_matrix = glm.rotate(model_matrix, rotation.x, glm.vec3(-1, 0, 0)) # x rotation
    model_matrix = glm.rotate(model_matrix, rotation.y, glm.vec3(0, -1, 0)) # y rotation
    model_matrix = glm.rotate(model_matrix, rotation.z, glm.vec3(0, 0, -1)) # z rotation
    model_matrix = glm.scale(model_matrix, scale) # scale
    
    return model_matrix

def get_rotation_matrix(rotation: glm.quat) -> glm.mat3x3: # 
    return glm.mat3x3()

# inertia tensors
def compute_inertia_moment(t:list[glm.vec3], i:int) -> float:
    return t[0][i] ** 2 + t[1][i] * t[2][i] + \
           t[1][i] ** 2 + t[0][i] * t[2][i] + \
           t[2][i] ** 2 + t[0][i] * t[1][i]
           
def compute_inertia_product(t:list[glm.vec3], i:int, j:int) -> float:
    return 2 * t[0][i] * t[0][j] + t[1][i] * t[2][j] + t[2][i] * t[1][j] + \
           2 * t[1][i] * t[1][j] + t[0][i] * t[2][j] + t[2][i] * t[0][j] + \
           2 * t[2][i] * t[2][j] + t[0][i] * t[1][j] + t[1][i] * t[0][j]