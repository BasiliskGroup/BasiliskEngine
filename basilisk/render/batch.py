import numpy as np
import moderngl as mgl


class Batch():
    chunk: ...
    """Reference to the parent chunk of the batch"""
    ctx: mgl.Context
    """Reference to the context of the parent engine"""
    program: mgl.Program
    """Reference to the program used by batches"""
    vao: mgl.VertexArray
    """The vertex array of the batch. Used for rendering"""
    vbo: mgl.Buffer
    """Buffer containing all the batch data"""

    def __init__(self, chunk) -> None:
        """
        Basilik batch object
        Contains all the data for a chunk batch to be stored and rendered
        """
        
        # Back references
        self.chunk = chunk
        self.ctx = chunk.chunk_handler.engine.ctx
        self.program = self.chunk.get_program()

        # Set intial values
        self.vbo = None
        self.vao = None

    def batch(self) -> bool:
        """
        Batches all the node meshes in the chunks bounds.
        Returns True if batch was successful.
        """

        self.program = self.chunk.get_program()

        # Empty list to contain all vertex data of models in the chunk
        batch_data = []

        # Loop through each node in the chunk, adding the nodes's mesh to batch_data
        index = 0
        for node in self.chunk.nodes:
            # Check that the node should be used
            if not node.mesh: continue
            if node.static != self.chunk.static: continue

            # Get the data from the node
            node_data = node.get_data()
            # Update the index
            node.data_index = index
            index += len(node_data)
            # Add to the chunk mesh
            batch_data.append(node_data)

        # Combine all meshes into a single array
        if len(batch_data) > 1: batch_data = np.vstack(batch_data)
        else: batch_data = np.array(batch_data, dtype='f4')


        # If there are no verticies, delete the chunk
        if len(batch_data) == 0: return False

        if self.vbo: self.vbo.release()
        if self.vao: self.vao.release()

        # Create the vbo and the vao from mesh data
        self.vbo = self.ctx.buffer(batch_data)
        self.vao = self.ctx.vertex_array(self.program, [(self.vbo, 
                                                         '3f 2f 3f 3f 3f 3f 4f 3f 1f', 
                                                         *['in_position', 'in_uv', 'in_normal', 'in_tangent', 'in_bitangent', 'obj_position', 'obj_rotation', 'obj_scale', 'obj_material'])], 
                                                         skip_errors=True)
        
        return True

    def __repr__(self) -> str:
        return f'<Basilisk Batch | {self.chunk.chunk_key}, {self.vbo.size / 1024  / 1024} mb>'

    def __del__(self) -> None:
        """
        Deallocates the mesh vbo and vao
        """
        
        if self.vbo: self.vbo.release()
        if self.vao: self.vao.release()