import numpy as np
from .model import Model
import glm

def from_data(data: np.ndarray, custom_format:bool=False) -> Model:
    """
    Converts data given to a format compatable with basilisk models
    Args:
        data: np.ndarray
            array of the mesh data
        custom_format: bool
            makes expected changes to the given data if false. Leaves data as given if true
    """

    if custom_format: return format_raw(data)

    # Create an empty model
    model = Model()

    # Get the shape of the given data and check for a valid shape
    shape = data.shape
    if len(shape) == 2: pass
    elif len(shape) == 3: data = np.reshape(data, (shape[0] * 3, shape[1] * shape[2] // 3)); shape = data.shape
    else: raise ValueError(f"Could not find valid format for the given model data of shape {shape}")

    # Data to be retraived/generated
    positions = None
    uvs       = None
    normals   = None
    tangents  = None

    if shape[1] == 3:  # Just given position
        positions = data[:,:]
        uvs = get_uvs(positions)
        normals = get_normals(positions)
        tangents = get_tangents(normals)

    elif shape[1] == 5:  # Given position and uv, but no normals
        positions = data[:,:3]
        uvs = data[:,3:5]
        normals = get_normals(positions)
        tangents = get_tangents(normals)

    elif shape[1] == 6:  # Given position and normals, but no UV
        positions = data[:,:3]
        uvs = get_uvs(positions)
        normals = data[:,3:6]
        tangents = get_tangents(normals)

    elif shape[1] == 8:  # Given position, normals and UV
        positions = data[:,:3]
        uvs = data[:,3:5]
        normals = data[:,5:8]
        tangents = get_tangents(normals)

    elif shape[1] == 14:  #Given position, normals, UV, bitangents, and tangents, no change needed
        positions = data[:,:3]
        uvs = data[:,3:5]
        normals = data[:,5:8]
        tangents = data[:,8:14]
    
    else:
        raise ValueError(f"Could not find valid format for the given model data of shape {shape}")

    model.vertex_data = np.zeros(shape=(shape[0], 14))
    model.vertex_data[:,:3]   = positions
    model.vertex_data[:,3:5]  = uvs
    model.vertex_data[:,5:8]  = normals
    model.vertex_data[:,8:14] = tangents
    model.vertex_points, model.point_indices = get_points_and_indices(positions)

    return model


def get_normals(positions: np.ndarray) -> np.ndarray:
    """
    Gets the normals from the position data
    Returns a numpy array
    """
    
    # Create empty array for the normals
    normals = np.zeros(shape=positions.shape, dtype='f4')

    # Loop through each triangle and calculate the normal of the surface
    for tri in range(positions.shape[0] // 3):
        v1 = glm.vec3(positions[tri * 3]) - glm.vec3(positions[tri * 3 + 1])
        v2 = glm.vec3(positions[tri * 3]) - glm.vec3(positions[tri * 3 + 2])
        normal = glm.normalize(glm.cross(v1, v2))
        normals[tri * 3    ] = list(normal.xyz)
        normals[tri * 3 + 1] = list(normal.xyz)
        normals[tri * 3 + 2] = list(normal.xyz)

    return normals

def get_uvs(positions: np.ndarray) -> np.ndarray:
    """
    Gets the uvs from the position array.
    Currently assigns each triangle arbitrarily, since there is no standard
    """
    uvs = np.array([*[[0, 0], [0, 1], [1, 0]] * (positions.shape[0]//3)])
    return uvs

def get_tangents(normals: np.array):
    """
    Gets the uvs from the normal array.
    Currently just fills with arbitrary data, since there is no standard
    """

    # Get linearly independent vectors
    tangent = np.cross(normals, [1, 1, 0])
    bitangent = np.cross(normals, tangent)

    # Combine to a single array
    all_tangents = np.hstack([tangent, bitangent], dtype='f4')

    return all_tangents


def get_points_and_indices(positions: np.ndarray) -> tuple[np.ndarray]:
    """
    Gets the unique points and indices from the position data.
    Returns a tuple of numpy arrays: (points, indices)
    """
    
    points = {}
    indices = [[] for i in range(len(positions) // 3)]

    for i, point in enumerate(positions):
        point = tuple(point)
        if point not in points: points[point] = []
        points[point].append(i // 3)

    for i, index_mapping in enumerate(points.values()):
        for triangle in index_mapping:
            indices[triangle].append(i)

    return np.array(list(points.keys()), dtype='f4'), np.array(indices, dtype='i')

def format_raw(data: np.ndarray) -> np.ndarray:
    # Create an empty model
    model = Model()

    # Get the needed data (assume position is in the first three positions)
    model.vertex_data = np.array(data, dtype='f4')
    if model.vertex_data.shape[1] >= 3:
        model.vertex_points, model.point_indices = get_points_and_indices(model.vertex_data[:,:3])
    else:
        model.vertex_points = np.zeros(shape=(1, model.vertex_data.shape[1]))
        model.point_indices = np.zeros(shape=(1, 3))

    return model