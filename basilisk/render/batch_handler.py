import numpy as np
import moderngl as mgl


class BatchHandler():
    engine: ...
    """Back reference to the parent engine"""
    scene: ...
    """Back reference to the parent scene"""
    ctx: mgl.Context
    """Back reference to the parent context"""
    
    def __init__(self, scene) -> None:
        # Reference to the scene hadlers and variables
        """
        Handles all the batching of all the nodes in the scene
        """
        
        # Back references
        self.scene   = scene
        self.engine  = scene.engine
        self.ctx     = scene.engine.ctx
        self.program = scene.shader_handler.programs['batch']

        # Contain lists with nodes positioned in a bounding box in space (Spatial partitioning)
        self.static_chunks  = {} 
        self.dynamic_chunks = {}
        # Contains VAOs for the chunk meshes
        self.static_batches  = {}
        self.dynamic_batches = {}
        # Chunks that need to have their mesh updated on the next frame
        self.static_updated_chunks = set()
        self.dynamic_updated_chunks = set()


    def render(self) -> None:
        """
        Renders all the chunk batches in the camera's range
        Includes some view culling, but not frustum culling. 
        """
        
        # Gets a rectanglur prism of chunks in the cameras view
        render_range_x, render_range_y, render_range_z = self.get_render_range()

        # Loop through all chunks in view and render
        for x in range(*render_range_x):
            for y in range(*render_range_y):
                for z in range(*render_range_z):
                    chunk = (x, y, z)  # Key of the chunk
                    
                    if chunk in self.static_batches:  self.static_batches[chunk].render()
                    if chunk in self.dynamic_batches: self.dynamic_batches[chunk].render()


    def update(self) -> None:           
        """
        Batches all the chunks that have been updated since the last frame. 
        """ 
        # Loop through the set of updated chunk keys and batch the chunk
        for chunk in self.static_updated_chunks:
            self.batch_chunk(chunk, static=True)
        for chunk in self.dynamic_updated_chunks:
            self.batch_chunk(chunk, static=False)

        # Clears the set of updated chunks so that they are not batched unless they are updated again
        self.static_updated_chunks.clear()
        self.dynamic_updated_chunks.clear()

    def batch_chunk(self, chunk_key: tuple, static=False) -> None:
        """
        Combines all the verticies of the chunk's nodes into a single VBO.
        This mesh can render the whole chunk in just one render call.
        Args:
            chunk_key: tuple = (x, y, z)
                The position of the chunk. Used as the key in the chunks and batches dicts
        """
        
        chunks = self.static_chunks if static else self.dynamic_chunks
        batches = self.static_batches if static else self.dynamic_batches

        # Get the chunks from key
        if chunk_key not in chunks: return
        chunk = chunks[chunk_key]

        # Store batched chunk mesh in the batches dict
        self.batches[chunk_key] = (vbo, vao)

    def get_render_range(self) -> tuple:
        """
        Returns a rectangluar prism of chunks that are in the camera's view.
        Tuple return is in form ((x1, x2), (y1, y2), (z1, z2))
        """
        
        cam_position = self.scene.camera.position  # glm.vec3(x, y, z)
        fov = 40  # The range in which a direction will not be culled

        # Default to a cube of chunks around the camera extending view_distance chunks in each direction
        chunk_size = self.engine.config.chunk_size
        render_distance = self.engine.config.render_distance
        render_range_x = [int(cam_position.x // chunk_size - render_distance), int(cam_position.x // chunk_size + render_distance + 1)]
        render_range_y = [int(cam_position.y // chunk_size - render_distance), int(cam_position.y // chunk_size + render_distance + 1)]
        render_range_z = [int(cam_position.z // chunk_size - render_distance), int(cam_position.z // chunk_size + render_distance + 1)]

        # Remove chunks that the camera is facing away from
        render_range_x[1] -= self.view_distance * (180 - fov < self.scene.camera.yaw < 180 + fov) - 1
        render_range_x[0] += self.view_distance * (-fov < self.scene.camera.yaw < fov or self.scene.camera.yaw > 360 - fov) - 1

        render_range_y[0] += self.view_distance * (self.scene.camera.pitch > 25) - 1
        render_range_y[1] -= self.view_distance * (self.scene.camera.pitch < -25) - 1

        render_range_z[1] -= self.view_distance * (270 - fov < self.scene.camera.yaw < 270 + fov) - 1
        render_range_z[0] += self.view_distance * (90 - fov < self.scene.camera.yaw < 90 + fov) - 1

        return (render_range_x, render_range_y, render_range_z)

    def add(self, node) -> None:
        """
        Add a an existing node to the correct chunk
        """

        # The key of the chunk the node will be added to
        chunk_size = self.engine.config.chunk_size
        chunk = (node.x // chunk_size, node.y // chunk_size, node.z // chunk_size)
        node.chunk = chunk

        # Create empty list if the chunk does not already exist
        chunks = self.static_chunks if node.static else self.dynamic_chunks
        if chunk not in chunks:
            chunks[chunk] = []

        # Add the node to the models list and to its correct chunk list
        chunks[chunk].append(node)

        # Update the chunk
        if node.static: self.static_updated_chunks.add(chunk)
        else:           self.dynamic_updated_chunks.add(chunk)

    def remove(self, node) -> None:
        """
        Removes an node from the batch handler
        """

        chunk = node.chunk
        if node in self.chunks[node.chunk]: self.chunks[node.chunk].remove(node)
        self.updated_chunks.add(chunk)