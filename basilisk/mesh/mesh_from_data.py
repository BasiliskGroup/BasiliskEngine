import numpy as np


def from_data(data: np.ndarray) -> np.ndarray:
    """
    Converts data given to a format compatable with basilisk models
    """

    shape = data.shape

    if shape[1] == 3:  # Just given position
        pos_norm_data = get_normals(data)
        print(pos_norm_data.shape)
        data = np.zeros(shape=(len(data), 14))
        data[:,:6] = pos_norm_data
        return data

    elif shape[1] == 6:  # Given position and normals, but no UV
        pos_norm_data = data
        data = np.zeros(shape=(len(data), 14))
        data[:][:6] = pos_norm_data
        return data

    elif shape[1] == 8:  # Given position, normals and UV
        ...

    elif shape[1] == 14:  #Given position, normals, UV, bitangents, and tangents, no change needed
        return data

    raise ValueError(f"Could not find valid format for the given model data of shape {shape}")


def get_normals(positions: np.ndarray) -> np.ndarray:
    """
    Gets the normals for a position array and returns a concatinated array
    """
    
    # Create empty array for the normals
    normals = np.zeros(shape=positions.shape)

    # Loop through each triangle and calculate the normal of the surface
    for tri in range(positions.shape[0] // 3):
        normal = np.cross(positions[tri] - positions[tri + 1], positions[tri] - positions[tri + 2])
        normals[tri    ] = normal
        normals[tri + 1] = normal
        normals[tri + 2] = normal

    return np.hstack([positions, normals])