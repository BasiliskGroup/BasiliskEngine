import numpy as np
import moderngl as mgl
from .chunk import Chunk
from ..nodes.node import Node


class ChunkHandler():
    engine: ...
    """Back reference to the parent engine"""
    scene: ...
    """Back reference to the parent scene"""
    ctx: mgl.Context
    """Back reference to the parent context"""
    program: mgl.Program
    """Reference to the shader program used by batches"""
    chunks: list[dict]
    """List containing two dictionaries for dynamic and static chunks repsectivly"""
    updated_chunks: set
    """Set containing recently updated chunks"""
    
    def __init__(self, scene) -> None:
        # Reference to the scene hadlers and variables
        """
        Handles all the chunking of all the nodes in the scene
        """
        
        # Back references
        self.scene   = scene
        self.engine  = scene.engine
        self.ctx     = scene.engine.ctx
        self.program = scene.engine.shader.program

        # List for the dynamic and static chunk dictionaries | [dyanmic: dict, static: dict]
        self.shader_groups = {None : ({}, {})}
        # self.chunks         = [{}, {}]
        self.updated_chunks = set()


    def render(self) -> None:
        """
        Renders all the chunk batches in the camera's range
        Includes some view culling, but not frustum culling. 
        """
        
        # Gets a rectanglur prism of chunks in the cameras view
        render_range_x, render_range_y, render_range_z = self.get_render_range()

        chunk_keys = [(x, y, z) for x in range(*render_range_x) for y in range(*render_range_y) for z in range(*render_range_z)]

        # Loop through all chunks in view and render
        for shader, group in self.shader_groups.items():
            if shader == None: shader = self.engine.shader            
            for chunk in chunk_keys:
                # Render the chunk if it exists
                if chunk in group[0]: group[0][chunk].render()
                if chunk in group[1]: group[1][chunk].render()


    def update(self) -> None:           
        """
        Updates all the chunks that have been updated since the last frame. 
        """ 

        self.program = self.scene.engine.shader.program

        # Loop through the set of updated chunk keys and update the chunk
        removes = []

        for chunk in self.updated_chunks: 
            if chunk.update(): continue
            removes.append(chunk)

        # Remove any empty chunks
        for chunk in removes:
            del self.shader_groups[chunk.shader][chunk.static][chunk.position]

        # Clears the set of updated chunks so that they are not updated unless they are updated again
        self.updated_chunks.clear()

    def update_all(self):
        self.program = self.scene.engine.shader.program
        for shader in self.shader_groups.values():
            for chunk in shader[0].values():
                self.updated_chunks.add(chunk)
            for chunk in shader[1].values():
                self.updated_chunks.add(chunk)

    def add(self, node: Node) -> Node:
        """
        Adds an existing node to its chunk. Updates the node's chunk reference
        """

        # The key of the chunk the node will be added to
        chunk_size = self.engine.config.chunk_size
        chunk_key = (int(node.x // chunk_size), int(node.y // chunk_size), int(node.z // chunk_size))
        shader = node.shader

        if shader not in self.shader_groups:
            self.shader_groups[shader] = ({}, {})

        # Ensure that the chunk exists
        if chunk_key not in self.shader_groups[shader][node.static]:
            chunk = Chunk(self, chunk_key, node.static, shader)
            self.shader_groups[shader][node.static][chunk_key] = chunk

        # Add the node to the chunk
        self.shader_groups[shader][node.static][chunk_key].add(node)

        # Update the chunk
        self.updated_chunks.add(self.shader_groups[shader][node.static][chunk_key])

        return Node

    def remove(self, node: Node) -> None:
        """
        Removes a node from the its chunk
        """

        if node == None: return

        # Remove the node
        chunk = node.chunk
        chunk.remove(node)
        node.chunk = None

        # Update the chunk
        self.updated_chunks.add(chunk)

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
        render_range_x[1] -= render_distance * (180 - fov < self.scene.camera.yaw < 180 + fov) - 1
        render_range_x[0] += render_distance * (-fov < self.scene.camera.yaw < fov or self.scene.camera.yaw > 360 - fov) - 1

        render_range_y[0] += render_distance * (self.scene.camera.pitch > 25) - 1
        render_range_y[1] -= render_distance * (self.scene.camera.pitch < -25) - 1

        render_range_z[1] -= render_distance * (270 - fov < self.scene.camera.yaw < 270 + fov) - 1
        render_range_z[0] += render_distance * (90 - fov < self.scene.camera.yaw < 90 + fov) - 1

        return (render_range_x, render_range_y, render_range_z)
    
    def swap_default(self, shader):
        """
        Swaps the shader of the default chunks
        """
        
        group = self.shader_groups[None]
        for chunk in group[0].values():
            chunk.swap_shader(shader)
        for chunk in group[1].values():
            chunk.swap_shader(shader)