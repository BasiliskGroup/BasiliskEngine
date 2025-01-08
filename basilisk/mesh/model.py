import numpy as np
import glm


class Model:
    """
    Instance of a loaded model. Contains all objects, groups, and vertex data
    model.vertex_data contains all vertex data
    Objects stored in the model.objects dictionary, where keys are the object names (marked by 'o') in the .obj
    Default object key is '0'
    """
    
    def __init__(self) -> None:
        self.objects = {0 : VertexObject()}

        self.vertex_data = []
        """All vertex data in the obj. Use this for buffer data"""
        self.tangent_data = []
        """Tangents and bitangents"""
        self.format  = None
        self.attribs = None

        self.vertex_points  = []
        """The unique points given by the file"""
        self.vertex_uv      = []
        """The unique texture coordinates given by the file"""
        self.vertex_normals = []
        """The unique normals given by the file"""

        self.point_indices  = []
        """Indices of to vertex_points to construct triangles. Grouped in three."""
        self.uv_indices     = []
        """Indices of to vertex_uv to construct triangles. Grouped in three."""
        self.normal_indices = []
        """Indices of to vertex_normals to construct triangles. Grouped in three."""


    def __repr__(self) -> str:
        return_string = '<Model | objects: {'
        for vertex_object in self.objects.keys():
            return_string += str(vertex_object) + ', '
        return_string = return_string[:-2] + '}>'
        return return_string


class VertexObject:
    """
    Object conataining groups of vertices.
    Groups stored in the vertex_object.groups dictionary, where keys are the group names (marked by 'g') in the .obj
    Default group key is '0'
    """
    
    def __init__(self) -> None:
        self.groups = {0 : VertexGroup()}

    def __repr__(self) -> str:
        return_string = '<Vertex Object | groups: {'
        for vertex_group in self.groups.keys():
            return_string += str(vertex_group) + ', '
        return_string = return_string[:-2] + '}>'
        return return_string


class VertexGroup:
    """
    Groups containing the vertex data
    vertex_group.vertex_data will be a numpy array of vertices
    """
    
    def __init__(self) -> None:
        self.vertex_data = []
        self.tangent_data = []

    def __repr__(self) -> str:
        return f'<Vertex Group | {self.vertex_data}>'


def load_model(obj_file: str, calculate_tangents=False) -> Model:
    """
    Loads an obj model. Returns a model class instance 
    model.vertex_data contains all vertex data combined in a single numpy array
    Args:
        file:
            Path to the .obj file to load
        calculate_tangents:
            Calculates the tangent and bitangent vectors for normal mapping
    """

    model = Model()
    current_object = 0
    current_group = 0

    vertex_format  = None
    vertex_attribs = None

    with open(obj_file, 'r') as file:
        line = file.readline()
        while line:
            line = line.strip()

            # Add object
            if line.startswith('o '):
                if line[2:].strip() not in model.objects:
                    model.objects[line[2:].strip()] = VertexObject()
                current_object = line[2:].strip()

            # Add group
            elif line.startswith('g '):
                if line[2:].strip() not in model.objects[current_object].groups:
                    model.objects[current_object].groups[line[2:].strip()] = VertexGroup()
                current_group = line[2:].strip()

            # Add vertex point
            elif line.startswith('v '):
                points = list(map(float, line[2:].strip().split(' ')))
                model.vertex_points.append(points)
            
            # Add vertex UV
            elif line.startswith('vt '):
                uvs = list(map(float, line[3:].strip().split(' ')))
                model.vertex_uv.append(uvs[:2])

            # Add vertex normals
            elif line.startswith('vn '):
                normals = list(map(float, line[3:].strip().split(' ')))
                model.vertex_normals.append(normals)

            # Create faces
            elif line.startswith('f '):
                corners = line[2:].strip().split(' ')
                # The index of the position, uv, and normal in each vertex
                vertex_indices = [[0, 0, 0] for i in range(len(corners))]
                for corner_index, corner in enumerate(corners):
                    corner = corner.split('/')

                    if not vertex_format:
                        if len(corner) == 1:
                            vertex_format  = '3f'
                            vertex_attribs = ['in_position']
                        elif not corner[1]:
                            vertex_format  = '3f 3f'
                            vertex_attribs = ['in_position', 'in_normal']
                        else:
                            vertex_format  = '3f 2f 3f'
                            vertex_attribs = ['in_position', 'in_uv', 'in_normal']

                    vertex = []

                    # Add each attribute to the vertex
                    for attribute, index in enumerate(corner):
                        if attribute == 0 and index:
                            vertex += model.vertex_points[int(index) - 1]
                            vertex_indices[corner_index][0] = int(index) - 1
                        if attribute == 1 and index:
                            vertex += model.vertex_uv[int(index) - 1]
                            vertex_indices[corner_index][1] = int(index) - 1
                        if attribute == 2 and index:
                            vertex += model.vertex_normals[int(index) - 1]
                            vertex_indices[corner_index][2] = int(index) - 1

                    # Replace the vertex data 
                    corners[corner_index] = vertex

                # Add each triangle to the objects vertex array
                for triangle in range(len(corners) - 2):
                    if 'in_normal' not in vertex_attribs: # If the model doesnt have normals, calculate face normals
                        p1 = glm.vec3(corners[0])
                        p2 = glm.vec3(corners[1 + triangle])
                        p3 = glm.vec3(corners[2 + triangle])
                        normal = glm.normalize(glm.cross(p2 - p1, p3 - p1))
                        normal = list(normal.xyz)
                        model.vertex_normals.append(normal)
                        model.objects[current_object].groups[current_group].vertex_data.append(corners[0] + normal)
                        model.objects[current_object].groups[current_group].vertex_data.append(corners[1 + triangle] + normal)
                        model.objects[current_object].groups[current_group].vertex_data.append(corners[2 + triangle] + normal)
                        
                        # Add the triangle to the indices
                        model.point_indices.append([vertex_indices[0][0], vertex_indices[1 + triangle][0], vertex_indices[2 + triangle][0]])
                        model.normal_indices.append([len(model.vertex_normals) - 1] * 3)
                    else: # Standard reading
                        model.objects[current_object].groups[current_group].vertex_data.append(corners[0])
                        model.objects[current_object].groups[current_group].vertex_data.append(corners[1 + triangle])
                        model.objects[current_object].groups[current_group].vertex_data.append(corners[2 + triangle])

                        # Add the triangle to the indices
                        model.point_indices.append([vertex_indices[0][0], vertex_indices[1 + triangle][0], vertex_indices[2 + triangle][0]])
                        model.uv_indices.append([vertex_indices[0][1], vertex_indices[1 + triangle][1], vertex_indices[2 + triangle][1]])
                        model.normal_indices.append([vertex_indices[0][2], vertex_indices[1 + triangle][2], vertex_indices[2 + triangle][2]])

                    # Calculate the tangents and bitangents
                    if calculate_tangents and 'in_uv' in vertex_attribs:
                        v1 = corners[0]
                        v2 = corners[1 + triangle]
                        v3 = corners[2 + triangle]

                        x1 = v2[0] - v1[0]
                        x2 = v3[0] - v1[0]
                        y1 = v2[1] - v1[1]
                        y2 = v3[1] - v1[1]
                        z1 = v2[2] - v1[2]
                        z2 = v3[2] - v1[2]
                        
                        s1 = v2[3] - v1[3]
                        s2 = v3[3] - v1[3]
                        t1 = v2[4] - v1[4]
                        t2 = v3[4] - v1[4]
                        
                        if (s1 * t2 - s2 * t1): r = 1.0 / (s1 * t2 - s2 * t1)
                        else: r = 1

                        tangent   = glm.normalize(((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r))
                        bitangent = glm.normalize(((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r))
            
                        T = np.array(tangent)
                        U = np.array(bitangent)
                        N1 = np.array(corners[0][5:8])
                        N2 = np.array(corners[1 + triangle][5:8])
                        N3 = np.array(corners[2 + triangle][5:8])

                        T1 = T - np.dot(N1, T) * N1
                        T2 = T - np.dot(N2, T) * N2
                        T3 = T - np.dot(N3, T) * N3
                        U1 = U - np.dot(N1, U) * N1 - np.dot(T1, U) * T1
                        U2 = U - np.dot(N2, U) * N2 - np.dot(T2, U) * T2
                        U3 = U - np.dot(N3, U) * N3 - np.dot(T3, U) * T3

                        data = [[*T1, *U1], [*T2, *U2], [*T3, *U3]]

                        model.objects[current_object].groups[current_group].tangent_data.extend(data)

            line = file.readline()

    vertex_groups = []
    tangent_groups = []

    # Loop through all vertex objects and groups in the model
    for object in model.objects.values():
        for group in object.groups.values():
            # Ignore empty groups
            if not len(group.vertex_data): continue
            # Convert to a numpy array
            group.vertex_data = np.array(group.vertex_data, dtype='f4')
            # Add to the vertex_groups list to be stacked
            vertex_groups.append(group.vertex_data)
            tangent_groups.append(group.tangent_data)

    # Array of all vertices from all the model's groups combined
    vertices = np.vstack(vertex_groups,  dtype='f4')
    tangents = np.vstack(tangent_groups, dtype='f4')

    # Save the model's combined vertices
    model.vertex_data  = vertices
    model.tangent_data = tangents

    # Convert the points and indices to array for convenience with C-like buffers
    model.vertex_points  = np.array(model.vertex_points,  dtype="f4")
    model.vertex_uv      = np.array(model.vertex_uv,      dtype="f4")
    model.vertex_normals = np.array(model.vertex_normals, dtype="f4")
    model.point_indices  = np.array(model.point_indices,  dtype="i4")
    model.uv_indices     = np.array(model.uv_indices,     dtype="i4")
    model.normal_indices = np.array(model.normal_indices, dtype="i4")

    # Add normals to position only models
    if vertex_format == '3f':
        vertex_format  = '3f 3f'
        vertex_attribs = ['in_position', 'in_normal']

    # Save the model vertex format and attribs
    model.format = vertex_format
    model.attribs = vertex_attribs

    return model