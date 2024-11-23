import os
import numpy as np
# from pyobjloader import load_model
from scripts.render.pyobjloader import load_model
from numba import njit
from uuid import uuid4


class VBOHandler:
    """
    Stores all vertex buffer objects
    """
    def __init__(self, ctx, directory='models'):
        self.ctx = ctx
        self.directory = directory
        self.vbos = {}
        # self.vbos['cube'] = CubeVBO(self.ctx)
        self.frame_vbo = FrameVBO(self.ctx)

    def release(self):
        """
        Releases all VBOs in handler
        """

        [vbo.vbo.release() for vbo in self.vbos.values()]
        
    def create_vbo(self, vertices, indices) -> str:
        """
        Creates a RuntimeVBO using the given data. 
        """
        uuid = str(uuid4())
        self.vbos[uuid] = RuntimeVBO(self.ctx, vertices, indices)
        return uuid

class BaseVBO:
    """
    Stores vertex data, format, and attributes for VBO
    """

    def __init__(self, ctx):
        self.ctx = ctx
        self.vbo = self.get_vbo()
        self.unique_points: list
        self.format: str = None
        self.attrib: list = None

    def get_vertex_data(self) -> np.ndarray: ...

    @staticmethod
    def get_data(verticies, indicies) -> np.ndarray:
        """
        Formats verticies based on indicies
        """
        data = [verticies[ind] for triangle in indicies for ind in triangle]
        return np.array(data, dtype='f4')

    def get_vbo(self):
        """
        Creates a buffer with the vertex data
        """
        
        self.vertex_data = self.get_vertex_data()
        vbo = self.ctx.buffer(self.vertex_data)

        unique_points_set = set()
        self.unique_points = []
        for x in self.vertex_data[:,:3].tolist():
            if tuple(x) not in unique_points_set:
                self.unique_points.append(x)
                unique_points_set.add(tuple(x))

        # Save the mash vertex indicies for softbody reconstruction
        self.mesh_indicies = np.zeros(shape=(len(self.vertex_data)))
        for i, vertex in enumerate(self.vertex_data):
            index = self.unique_points.index(vertex[:3].tolist())
            self.mesh_indicies[i] = index

        self.unique_points = np.array(self.unique_points, dtype='f4')

        return vbo
    
class CubeVBO(BaseVBO):
    def __init__(self, ctx):
        super().__init__(ctx)
        self.format = '3f 2f 3f'
        self.attribs = ['in_position', 'in_uv', 'in_normal']

    def get_vertex_data(self):
        verticies = [(-1, -1, 1), ( 1, -1,  1), (1,  1,  1), (-1, 1,  1),
                     (-1,  1,-1), (-1, -1, -1), (1, -1, -1), ( 1, 1, -1)]
        
        self.indicies = [(0, 2, 3), (0, 1, 2),
                    (1, 7, 2), (1, 6, 7),
                    (6, 5, 4), (4, 7, 6),
                    (3, 4, 5), (3, 5, 0),
                    (3, 7, 4), (3, 2, 7),
                    (0, 6, 1), (0, 5, 6)]

        vertex_data = self.get_data(verticies, self.indicies)

        tex_coord_verticies = [(0, 0), (1, 0), (1, 1), (0, 1)]
        tex_coord_indicies = [(0, 2, 3), (0, 1, 2),
                              (0, 2, 3), (0, 1, 2),
                              (0, 1, 2), (2, 3, 0),
                              (2, 3, 0), (2, 0, 1),
                              (0, 2, 3), (0, 1, 2),
                              (3, 1, 2), (3, 0, 1)]
        
        tex_coord_data = self.get_data(tex_coord_verticies, tex_coord_indicies)

        normals = [(0, 0,  1) * 6,
                   ( 1, 0, 0) * 6,
                   (0, 0, -1) * 6,
                   (-1, 0, 0) * 6,
                   (0,  1, 0) * 6,
                   (0, -1, 0) * 6]
        normals = np.array(normals, dtype='f4').reshape(36, 3)

        vertex_data = np.hstack([vertex_data, tex_coord_data])
        vertex_data = np.hstack([vertex_data, normals])
        return vertex_data


class PlaneVBO(BaseVBO):
    def __init__(self, ctx):
        super().__init__(ctx)
        self.format = '3f 2f'
        self.attribs = ['in_position', 'in_texcoord']

    def get_vertex_data(self):
        verticies = np.array([  [-1, 0, -1],  # Bottom Left
                                [ 1, 0, -1],  # Bottom Right
                                [ 1, 0,  1],  # Top Right
                                [-1, 0,  1],  # Top Left
                                ])
        indicies = [(3, 0, 1),
                    (2, 3, 1)]

        vertex_data = self.get_data(verticies, indicies)

        tex_coord_verticies =   [
                                (0, 0), # Bottom Left
                                (1, 0), # Bottom Right
                                (1, 1), # Top Right
                                (0, 1)  # Top Left
                                ]
        tex_coord_indicies = [(3, 0, 1),
                              (2, 3, 1)]
        tex_coord_data = self.get_data(tex_coord_verticies, tex_coord_indicies)


        vertex_data = np.hstack([vertex_data, tex_coord_data])
        return vertex_data


class FrameVBO(BaseVBO):
    def __init__(self, ctx):
        super().__init__(ctx)
        self.format = '3f 2f'
        self.attribs = ['in_position', 'in_uv']

    def get_vertex_data(self):
        verticies = np.array([[-1, -1, 0],  # Bottom Left
                     [ 1, -1, 0],  # Bottom Right
                     [ 1, 1, 0],   # Top Right
                     [-1, 1, 0],  # Top Left
                     ])
        indicies = [(3, 0, 1),
                    (2, 3, 1)]

        vertex_data = self.get_data(verticies, indicies)

        tex_coord_verticies =   [
                                (0, 0), # Bottom Left
                                (1, 0), # Bottom Right
                                (1, 1), # Top Right
                                (0, 1)  # Top Left
                                ]
        tex_coord_indicies = [(3, 0, 1),
                              (2, 3, 1)]
        tex_coord_data = self.get_data(tex_coord_verticies, tex_coord_indicies)


        vertex_data = np.hstack([vertex_data, tex_coord_data])
        return vertex_data
    

class ModelVBO(BaseVBO):
    def __init__(self, ctx, path):
        self.path = path
        super().__init__(ctx)
        self.format = self.model.format
        self.attribs = self.model.attribs
        self.triangles = None

    def get_vbo(self):
        """
        Creates a buffer with the vertex data
        """
        
        self.vertex_data = self.get_vertex_data()
        vbo = self.ctx.buffer(self.vertex_data)
        
        self.unique_points = self.model.vertex_points
        self.indicies = self.model.point_indices

        return vbo

    def get_vertex_data(self):
        self.model = load_model(self.path, calculate_tangents=True)

        if len(self.model.vertex_data[0]) == 8:
            vertex_data = self.model.vertex_data.copy()
        else:
            vertex_data = np.zeros(shape=(len(self.model.vertex_data), 8))
            vertex_data[:,:3] = self.model.vertex_data[:,:3]
            vertex_data[:,5:] = self.model.vertex_data[:,3:]
        
        if len(self.model.tangent_data[0]) == 6:
            vertex_data = np.hstack([vertex_data, self.model.tangent_data])
        else:
            tangents = np.zeros(shape=(len(vertex_data), 6))
            tangents[:,:] += [1.0, 0.0, 0.0, 0.0, 1.0, 0.0]
            vertex_data = np.hstack([vertex_data, tangents])

        return vertex_data
    
class RuntimeVBO(BaseVBO):
    def __init__(self, ctx, unique_points, indicies):
        self.unique_points = unique_points
        self.indicies = [tuple(i) for i in indicies]
        super().__init__(ctx)
        self.format = '3f 2f 3f 3f 3f'
        self.attribs = ['in_position', 'in_uv', 'in_normal', 'in_tangent', 'in_bitangent']
        self.unique_points = unique_points
        
    def get_vertex_data(self):

        vertex_data = self.get_data(self.unique_points, self.indicies)

        # faked data - we cannot use textures or normals
        tex_coord_verticies = [(0, 0), (1, 0), (1, 1), (0, 1)]
        tex_coord_indicies = [(0, 1, 2) for _ in range(len(self.indicies))]
        tex_coord_data = self.get_data(tex_coord_verticies, tex_coord_indicies)
        

        # calculate normal
        normals = []
        for i, triangle in enumerate(self.indicies):
            points  = [np.array(self.unique_points[triangle[i]]) for i in range(3)]
            normal  = np.cross(points[1] - points[0], points[2] - points[0])
            normal /= np.linalg.norm(normal)
            normal  = normal.tolist()
            
            normals.extend([normal for _ in range(3)])
            
        normals = np.array(normals, dtype='f4').reshape(len(self.indicies) * 3, 3)
        
        tangents = [(1, 0, 0) for _ in range(len(normals))]
        bitangents = [(0, 1, 0) for _ in range(len(normals))]

        vertex_data = np.hstack([vertex_data, tex_coord_data])
        vertex_data = np.hstack([vertex_data, normals])
        vertex_data = np.hstack([vertex_data, tangents])
        vertex_data = np.hstack([vertex_data, bitangents])
        
        return vertex_data