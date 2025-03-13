import glm
from .node import Node
from .helper import node_is
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
        self.engine = scene.engine
        self.nodes = []
        self.chunk_handler = ChunkHandler(scene)

    def update(self):
        """
        Updates the nodes and chunks in the scene
        """
        dt = self.scene.engine.delta_time
        if dt < 0.5:
            for node in self.nodes: 
                # if not node.static: TODO determine better solution to this line
                    node.update(dt)
        self.chunk_handler.update()

    def render(self):
        """
        Updates the node meshes in the scene
        """
        
        self.chunk_handler.render()

    def add(self, node: Node) -> Node:
        """
        Adds a new node to the node handler
        """
        if node in self.nodes: return
        
        for n in node.get_all(): # gets all nodes including the node to be added
            
            # Update scene Handlers
            self.engine.shader_handler.add(n.shader)
            if not n.material: n.material = self.engine.material_handler.base
            self.engine.material_handler.add(n.material)
            
            # Update the node attributes
            n.init_scene(self.scene)
            
            # Add the node to internal data
            self.nodes.append(n)
            self.chunk_handler.add(n)

        return node
        
    def get(self, position: glm.vec3=None, scale: glm.vec3=None, rotation: glm.quat=None, forward: glm.vec3=None, mesh: Mesh=None, material: Material=None, velocity: glm.vec3=None, rotational_velocity: glm.quat=None, physics: bool=None, mass: float=None, collisions: bool=None, static_friction: float=None, kinetic_friction: float=None, elasticity: float=None, collision_group: float=None, name: str=None, tags: list[str]=None,static: bool=None) -> Node:
        """
        Returns the first node with the given traits
        """
        for node in self.nodes:
            if node_is(node, position, scale, rotation, forward, mesh, material, velocity, rotational_velocity, physics, mass, collisions, static_friction, kinetic_friction, elasticity, collision_group, name, tags, static): return node
        return None
    
    def get_all(self, position: glm.vec3=None, scale: glm.vec3=None, rotation: glm.quat=None, forward: glm.vec3=None, mesh: Mesh=None, material: Material=None, velocity: glm.vec3=None, rotational_velocity: glm.quat=None, physics: bool=None, mass: float=None, collisions: bool=None, static_friction: float=None, kinetic_friction: float=None, elasticity: float=None, collision_group: float=None, name: str=None, tags: list[str]=None,static: bool=None) -> list[Node]:
        """
        Returns all nodes with the given traits
        """
        nodes = []
        for node in self.nodes:
            if node_is(node, position, scale, rotation, forward, mesh, material, velocity, rotational_velocity, physics, mass, collisions, static_friction, kinetic_friction, elasticity, collision_group, name, tags, static): nodes.append(node)
        return nodes
    
    def remove(self, node: Node) -> None: 
        """
        Removes a node and all of its children from their handlers
        """

        if node == None: return

        # TODO add support for recursive nodes
        if node in self.nodes:
            if node.physics_body: self.scene.physics_engine.remove(node.physics_body)
            if node.collider: self.scene.collider_handler.remove(node.collider)
            self.chunk_handler.remove(node)
            self.nodes.remove(node)
            node.node_handler = None
            
        for child in node.children: self.remove(child)