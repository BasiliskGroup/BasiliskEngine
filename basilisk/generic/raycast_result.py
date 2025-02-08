import glm
from ..nodes.node import Node


class RaycastResult:
    node: Node | None
    """The node that the raycast hit. Is None if no object was hit"""
    position: glm.vec3
    """The node that the raycast hit"""
    normal: glm.vec3
    """The normal of the raycast hit"""

    def __init__(self, node: Node | None, position: glm.vec3, normal: glm.vec3):
        """
        Container for returning raycast results.
        Contains the node hit and the global position the raycast hit at.
        """
        
        self.node = node
        self.position = position
        self.normal = normal

    def __bool__(self):
        return bool(self.node)

    def __repr__(self):
        return f'<Raycast | Node: {self.node}, Position: {self.position}, Normal: {self.normal}>'