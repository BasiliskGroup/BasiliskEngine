from .batch import Batch


class Chunk():
    chunk_handler: ...
    """Back refrence to the parent chunk handler"""
    chunk_key: tuple
    """The position of the chunk. Used as a key in the chunk handler"""
    batch: Batch
    """Batched mesh of the chunk"""
    nodes: set
    """Set conaining references to all nodes in the chunk"""
    static: bool
    """Type of node that the chunk recognizes"""

    def __init__(self, chunk_handler, chunk_key: tuple, static: bool) -> None:
        """
        Basilisk chunk object. 
        Contains references to all nodes in the chunk.
        Handles batching for its own nodes
        """

        # Back references
        self.chunk_handler = chunk_handler
        self.chunk_key = chunk_key

        self.static = static

        # Create empty batch
        self.batch = Batch(self)

        # Create empty set for chunk's nodes
        self.nodes = set()

    def render(self) -> None:
        """
        Renders the chunk mesh
        """

        if self.batch.vao: self.batch.vao.render()

    def update(self) -> bool:
        """
        Batches all the node meshes in the chunk        
        """

        # Check if there are no nodes in the chunk
        if not self.nodes: return False
        # Batch the chunk nodes, return success bit
        return self.batch.batch()

    def node_update_callback(self, node):
        if not self.batch.vbo: return
        
        data = node.get_data()
        self.batch.vbo.write(data, node.data_index * 25 * 4)

    def add(self, node):
        """
        Adds an existing node to the chunk. Updates the node's chunk reference
        """

        self.nodes.add(node)
        node.chunk = self

        return node

    def remove(self, node):
        """
        Removes a node from the chunk
        """

        self.nodes.remove(node)
        self.batch.vbo.clear()

        return node

    def __repr__(self) -> str:
        return f'<Basilisk Chunk | {self.chunk_key}, {len(self.nodes)} nodes, {'static' if self.static else 'dynamic'}>'

    def __del__(self) -> None:
        """
        Deletes the batch if this chunk is deleted
        """
        
        del self.batch