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
        self.format  = None
        self.attribs = None

        self.vertex_points = []
        self.vertex_uv = []
        self.vertex_normals = []

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

    def __repr__(self) -> str:
        return f'<Vertex Group | {self.vertex_data}>'


def load_model(obj_file: str) -> Model:
    """
    Loads an obj model. Returns a model class instance 
    model.vertex_data contains all vertex data combined in a single numpy array
    Args:
        file: Path to the .obj file to load
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
                        if attribute == 1 and index:
                            vertex += model.vertex_uv[int(index) - 1]
                        if attribute == 2 and index:
                            vertex += model.vertex_normals[int(index) - 1]

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
                        model.objects[current_object].groups[current_group].vertex_data.append(corners[0] + normal)
                        model.objects[current_object].groups[current_group].vertex_data.append(corners[1 + triangle] + normal)
                        model.objects[current_object].groups[current_group].vertex_data.append(corners[2 + triangle] + normal)
                    else: # Standard reading
                        model.objects[current_object].groups[current_group].vertex_data.append(corners[0])
                        model.objects[current_object].groups[current_group].vertex_data.append(corners[1 + triangle])
                        model.objects[current_object].groups[current_group].vertex_data.append(corners[2 + triangle])

            line = file.readline()

    vertex_groups = []

    # Loop through all vertex objects and groups in the model
    for object in model.objects.values():
        for group in object.groups.values():
            # Ignore empty groups
            if not len(group.vertex_data): continue
            # Convert to a numpy array
            group.vertex_data = np.array(group.vertex_data, dtype='f4')
            # Add to the vertex_groups list to be stacked
            vertex_groups.append(group.vertex_data)

    # Array of all vertices from all the model's groups combined
    vertices = np.vstack(vertex_groups, dtype='f4')

    # Save the model's combined vertices
    model.vertex_data = vertices

    # Add normals to position only models
    if vertex_format == '3f':
        vertex_format  = '3f 3f'
        vertex_attribs = ['in_position', 'in_normal']

    # Save the model vertex format and attribs
    model.format = vertex_format
    model.attribs = vertex_attribs

    return model