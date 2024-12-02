import numpy as np
import glm
import os
from pyobjloader import load_model
from ..generic.matrices import compute_inertia_moment, compute_inertia_product

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

    def __init__(self, path: str | os.PathLike) -> None:
        """
        Mesh object containing all the data needed to render an object and perform physics/collisions on it
        Args:
            path: str
                path to the .obj file of the model
        """
        
        # Verify the path type
        if not isinstance(path, str) and not isinstance(path, os.PathLike):
            raise TypeError(f'Invalid path type: {type(path)}. Expected a string or os.path')

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
        
        # generate geometric data (caclulated assuming scale = (1, 1, 1) and density = 1)
        self.volume = 0
        self.geometric_center = 0
        self.center_of_mass = 0
        
        ia = ib = ic = iap = ibp = icp = 0
        for triangle in self.indices:
            pts = [self.points[t] for t in triangle]
            det_j = glm.dot(pts[0], glm.cross(pts[1], pts[2]))
            tet_volume = det_j / 6
            
            ia += det_j * (compute_inertia_moment(pts, 1) + compute_inertia_moment(pts, 2))
            ib += det_j * (compute_inertia_moment(pts, 0) + compute_inertia_moment(pts, 2))
            ic += det_j * (compute_inertia_moment(pts, 0) + compute_inertia_moment(pts, 1))
            iap += det_j * compute_inertia_product(pts, 1, 2)
            ibp += det_j * compute_inertia_product(pts, 0, 1)
            icp += det_j * compute_inertia_product(pts, 0, 2)
            
            self.volume += tet_volume
            self.center_of_mass += tet_volume * (pts[0] + pts[1] + pts[2]) / 4
            
        self.center_of_mass /= self.volume
        ia = ia / 60 - self.volume * (self.center_of_mass[1] ** 2 + self.center_of_mass[2] ** 2)
        ib = ib / 60 - self.volume * (self.center_of_mass[0] ** 2 + self.center_of_mass[2] ** 2)
        ic = ic / 60 - self.volume * (self.center_of_mass[0] ** 2 + self.center_of_mass[1] ** 2)
        iap = iap / 120 - self.volume * self.center_of_mass[1] * self.center_of_mass[2]
        ibp = ibp / 120 - self.volume * self.center_of_mass[0] * self.center_of_mass[1]
        icp = icp / 120 - self.volume * self.center_of_mass[0] * self.center_of_mass[2]
        
        self.inertia_tensor = glm.mat3x3(
             ia,  -ibp, -icp,
            -ibp,  ib,  -iap,
            -icp, -iap,  ic
        )

    def __repr__(self) -> str:
        size = (self.data.nbytes + self.points.nbytes + self.indices.nbytes) / 1024 / 1024
        return f'<Basilisk Mesh | {len(self.data)} vertices, {size:.2} mb>'