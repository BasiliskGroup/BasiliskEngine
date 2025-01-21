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
            position:            glm.vec3 =None, 
            scale:               glm.vec3=None, 
            rotation:            glm.quat=None, 
            relative_position:   bool=True,
            relative_scale:      bool=True,
            relative_rotation:   bool=True,
            forward:             glm.vec3=None, 
            mesh:                Mesh=None, 
            material:            Material=None, 
            velocity:            glm.vec3=None, 
            rotational_velocity: glm.vec3=None, 
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
            static:              bool=None,
            shader:              ...=None
        ) -> Node:
        """
        Adds a new node to the node handler
        """
        
        node = Node(self, position, scale, rotation, relative_position, relative_scale, relative_rotation, forward, mesh, material, velocity, rotational_velocity, physics, mass, collisions, collider, static_friction, kinetic_friction, elasticity, collision_group, name, tags, static, shader)

        self.nodes.append(node)
        self.chunk_handler.add(node)

        return node
    
    def node_is(self, node: Node, position: glm.vec3=None, scale: glm.vec3=None, rotation: glm.quat=None, forward: glm.vec3=None, mesh: Mesh=None, material: Material=None, velocity: glm.vec3=None, rotational_velocity: glm.quat=None, physics: bool=None, mass: float=None, collisions: bool=None, static_friction: float=None, kinetic_friction: float=None, elasticity: float=None, collision_group: float=None, name: str=None, tags: list[str]=None,static: bool=None) -> bool:
        """
        Determine if a node meets the requirements given by the parameters. If a parameter is None, then the filter is not applied.
        """
        return all([
                position is None or position == node.position,
                scale    is None or scale    == node.scale,
                rotation is None or rotation == node.rotation,
                forward  is None or forward  == node.forward,
                mesh     is None or mesh     == node.mesh,
                material is None or material == node.material,
                velocity is None or velocity == node.velocity,
                rotational_velocity is None or rotational_velocity == node.rotational_velocity,
                physics    is None or node.physics_body,
                mass       is None or (node.physics_body and mass == node.physics_body.mass),
                collisions is None or node.collider,
                static_friction  is None or (node.collider and node.collider.static_friction  == static_friction),
                kinetic_friction is None or (node.collider and node.collider.kinetic_friction == kinetic_friction),
                elasticity       is None or (node.collider and node.collider.elasticity       == elasticity),
                collision_group  is None or (node.collider and node.collider.collision_group  == collision_group),
                name   is None or node.name == name,
                tags   is None or all([tag in node.tags for tag in tags]),
                static is None or node.static == static
            ])
        
    def get(self, position: glm.vec3=None, scale: glm.vec3=None, rotation: glm.quat=None, forward: glm.vec3=None, mesh: Mesh=None, material: Material=None, velocity: glm.vec3=None, rotational_velocity: glm.quat=None, physics: bool=None, mass: float=None, collisions: bool=None, static_friction: float=None, kinetic_friction: float=None, elasticity: float=None, collision_group: float=None, name: str=None, tags: list[str]=None,static: bool=None) -> Node:
        """
        Returns the first node with the given traits
        """
        for node in self.nodes:
            if self.node_is(node, position, scale, rotation, forward, mesh, material, velocity, rotational_velocity, physics, mass, collisions, static_friction, kinetic_friction, elasticity, collision_group, name, tags, static): return node
        return None
    
    def get_all(self, position: glm.vec3=None, scale: glm.vec3=None, rotation: glm.quat=None, forward: glm.vec3=None, mesh: Mesh=None, material: Material=None, velocity: glm.vec3=None, rotational_velocity: glm.quat=None, physics: bool=None, mass: float=None, collisions: bool=None, static_friction: float=None, kinetic_friction: float=None, elasticity: float=None, collision_group: float=None, name: str=None, tags: list[str]=None,static: bool=None) -> Node:
        """
        Returns all nodes with the given traits
        """
        nodes = []
        for node in self.nodes:
            if self.node_is(node, position, scale, rotation, forward, mesh, material, velocity, rotational_velocity, physics, mass, collisions, static_friction, kinetic_friction, elasticity, collision_group, name, tags, static): nodes.append(node)
        return nodes
    
    def remove(self, node: Node) -> None: 
        """
        Removes a node and all of its children from their handlers
        """
        # TODO add support for recursive nodes
        if node in self.nodes:
            if node.physics_body: self.scene.physics_engine.remove(node.physics_body)
            if node.collider: self.scene.collider_handler.remove(node.collider)
            self.chunk_handler.remove(node)
            self.nodes.remove(node)
            node.node_handler = None