from .node import Node

class NodeHandler():
    scene: ...
    """Back reference to the scene"""
    nodes: list[Node]
    """The list of root nodes in the scene"""
    
    def __init__(self, scene):
        self.scene = scene
        self.nodes = []
        