import numpy as np
from .model import Model
import glm


def from_data(data: np.ndarray) -> Model:
    """
    Converts data given to a format compatable with basilisk models
    """

    model = Model()

    shape = data.shape

    if shape[1] == 3:  # Just given position
        norms = get_normals(data)
        all_data = np.zeros(shape=(len(data), 14))
        all_data[:,:3] = data
        all_data[:,5:8] = norms
        data = all_data

    elif shape[1] == 6:  # Given position and normals, but no UV
        all_data = np.zeros(shape=(len(data), 14))
        all_data[:,:3] = data[:,:3]
        all_data[:,5:8] = data[:,3:]
        data = all_data

    elif shape[1] == 8:  # Given position, normals and UV
        all_data = np.zeros(shape=(len(data), 14))
        all_data[:,:8] = data[:,:8]
        all_data[:,8] = 1.0
        all_data[:,13] = 1.0
        data = all_data

    elif shape[1] == 14:  #Given position, normals, UV, bitangents, and tangents, no change needed
        ...

    else:
        raise ValueError(f"Could not find valid format for the given model data of shape {shape}")

    # Save the model's combined vertices
    model.vertex_data  = data

    model.vertex_points = np.array(list(set(map(tuple, data[:,:3]))))
    model.point_indices = np.array([[0, 0, 0]])

    return model


def get_normals(positions: np.ndarray) -> np.ndarray:
    """
    Gets the normals for a position array and returns a concatinated array
    """
    
    # Create empty array for the normals
    normals = np.zeros(shape=positions.shape)

    # Loop through each triangle and calculate the normal of the surface
    for tri in range(positions.shape[0] // 3):
        v1 = glm.vec3(positions[tri * 3]) - glm.vec3(positions[tri * 3 + 1])
        v2 = glm.vec3(positions[tri * 3]) - glm.vec3(positions[tri * 3 + 2])
        normal = glm.normalize(glm.cross(v1, v2))
        normals[tri * 3    ] = list(normal.xyz)
        normals[tri * 3 + 1] = list(normal.xyz)
        normals[tri * 3 + 2] = list(normal.xyz)

    return normals