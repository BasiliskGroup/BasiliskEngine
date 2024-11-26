import numpy as np
import glm
from pyobjloader import load_model

class Mesh():
    data: np.ndarray
    """The mesh vertex data stored as a 4-byte float numpy array. Format will be [position.xyz, uv.xy, normal.xyz, tangent.xyz, bitangent.xyz]"""
    points: np.ndarray
    """All the unique points of the mesh given by the model file"""  
    indices: np.ndarray
    """Indices of the triangles corresponding to the points array"""  
    inertia_tensor: glm.mat3x3
    """Stores the mass distribution of an object as a matrix"""
    bvh: any
    """Data structure allowing the access of closest points more efficiently"""
    volume: float
    """The volume of the unscaled mesh"""
    geometric_center: glm.vec3
    """The geometric center of the mesh"""
    center_of_mass: glm.vec3
    """The center of mass of the mesh calculated from the inertia tensor algorithm"""

    def __init__(self, path: str) -> None:
        """
        Mesh object containing all the data needed to render an object and perform physics/collisions on it
        Args:
            path: str
                path to the .obj file of the model
        """
        
        # Load the model from file
        model = load_model(path)


        # Get the vertex data
        if len(model.vertex_data[0]) == 8:
            self.data = model.vertex_data.copy()
        else:
            self.data = np.zeros(shape=(len(model.vertex_data), 8))
            self.data[:,:3] = model.vertex_data[:,:3]
            self.data[:,5:] = model.vertex_data[:,3:]
        
        # Get tangent data
        if len(model.tangent_data[0]) == 6:
            self.data = np.hstack([self.data, model.tangent_data])
        else:
            tangents = np.zeros(shape=(len(self.data), 6))
            tangents[:,:] += [1.0, 0.0, 0.0, 0.0, 1.0, 0.0]
            self.data = np.hstack([self.data, tangents])


        # Mesh points and triangles used for physics/collisions
        self.points = model.vertex_points.copy()
        self.indices = model.point_indices.copy()

        # Model will no longer be used
        del model
