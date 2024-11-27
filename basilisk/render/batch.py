import numpy as np
import moderngl as mgl


class Batch():
    vao: mgl.VertexArray
    """The vertex array of the batch. Used for rendering"""
    vbo: mgl.Buffer
    """Buffer containing all the batch data"""

    def __init__(self, chunk: list, static=False) -> None:
        """
        Basilik batch object
        Contains all the data for a chunk batch to be stored and rendered
        """
        
        # Empty list to contain all vertex data of models in the chunk
        batch_data = []

        # Loop through each node in the chunk, adding the nodes's mesh to batch_data
        for node in chunk:
            # Check that the node should be used
            if not node.mesh: continue
            if node.static != static: continue
            # Get data from the node and mesh
            vertex_data = node.mesh.data
            model_data = np.array([*node.position, *node.rotation, *node.scale, node.material.index])

            # Create an array to hold the node's data
            object_data = np.zeros(shape=(vertex_data.shape[0], 24), dtype='f4')
            object_data[:,:14] = vertex_data
            object_data[:,14:] = model_data

            # Add to the chunk mesh
            batch_data.append(object_data)

        # Combine all meshes into a single array
        if len(batch_data) > 1: batch_data = np.vstack(batch_data)
        else: batch_data = np.array(batch_data, dtype='f4')

        # If there are no verticies, delete the chunk
        if len(batch_data) == 0:
            if chunk_key in chunks:
                del chunks[chunk_key]
            if chunk_key in batches: 
                batches[chunk_key].release()
                del batches[chunk_key]
            return

        # Release any existing chunk mesh
        if chunk_key in batches: batches[chunk_key].release()

        # Create the vbo and the vao from mesh data
        vbo = self.ctx.buffer(batch_data)
        vao = self.ctx.vertex_array(self.program, [(vbo, '3f 2f 3f 3f 3f 3f 3f 3f 1f', *['in_position', 'in_uv', 'in_normal', 'in_tangent', 'in_bitangent', 'obj_position', 'obj_rotation', 'obj_scale', 'obj_material'])], skip_errors=True)
