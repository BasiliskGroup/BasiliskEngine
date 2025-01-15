import glm
from .node import Node
from ..render.chunk_handler import ChunkHandler
from ..mesh.mesh import Mesh
from ..render.material import Material


class NodeHandler():
    scene: ...
    """Back reference to the scene"""
    nodes: list[Node]
    """The list of root nodes in the scene"""
    
    def __init__(self, scene):
        """
        Contains all the nodes in the scene.
        Handles chunking and batching of nodes
        """
        
        self.scene = scene
        self.nodes = []
        self.chunk_handler = ChunkHandler(scene)

    def update(self):
        """
        Updates the nodes and chunks in the scene
        """
        for node in self.nodes: 
            if node.static: continue
            node.update(self.scene.engine.delta_time)
        self.chunk_handler.update()

    def render(self):
        """
        Updates the node meshes in the scene
        """
        
        self.chunk_handler.render()

    def add(self, 
            position:            glm.vec3=None, 
            scale:               glm.vec3=None, 
            rotation:            glm.quat=None, 
            forward:             glm.vec3=None, 
            mesh:                Mesh=None, 
            material:            Material=None, 
            velocity:            glm.vec3=None, 
            rotational_velocity: glm.quat=None, 
            physics:             bool=False, 
            mass:                float=None, 
            collisions:          bool=False, 
            collider:            str=None, 
            static_friction:     float=None, 
            kinetic_friction:    float=None, 
            elasticity:          float=None, 
            collision_group :    float=None, 
            name:                str='', 
            tags:                list[str]=None,
            static:              bool=True
        ) -> Node:
        """
        Adds a new node to the node handler
        """
        
        node = Node(self, position, scale, rotation, forward, mesh, material, velocity, rotational_velocity, physics, mass, collisions, collider, static_friction, kinetic_friction, elasticity, collision_group, name, tags, static)

        self.nodes.append(node)
        self.chunk_handler.add(node)

        return node
        
    def get(self, name: str) -> Node: # TODO switch to full filter and adapt to search roots
        """
        Returns the first node with the given traits
        """
        for node in self.nodes:
            if node.name == name: return node
        return None
    
    def get_all(self, name: str) -> Node:
        """
        Returns all nodes with the given traits
        """
        nodes = []
        for node in self.nodes:
            if node.name == name: nodes.append(node)
        return nodes
    
    def remove(self, node: Node) -> None: 
        """
        Removes a node and all of its children from their handlers
        """
        # TODO add support for recursive nodes
        if node in self.nodes:
            if node.physics_body: self.scene.physics_engine.remove(node.physics_body)
            if node.collider: self.scene.collider_handler.remove(node.collider)
            node.node_handler = None
            self.nodes.remove(node)