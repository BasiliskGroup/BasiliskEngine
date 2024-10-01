import glm
from scripts.nodes.node import Node
import time

# handles updating nodes
class NodeHandler():
    def __init__(self, scene, nodes = None, death_plane:float=-100):
        self.scene       = scene
        self.death_plane = death_plane
        self.nodes = nodes if nodes else []
        
        # game ticks
        self.tick_iterval = 1/20
        self.tick_time = 0
        
    def update(self, delta_time:float):
        """
        Updates all top level nodes.
        """
        # update nodes
        ticked = False
        self.tick_time += time.time()
        if self.tick_time > self.tick_iterval:
            self.tick_time = 0
            ticked = True
            
        for node in self.nodes: node.update(delta_time, ticked)
    
    # create and add to top level nodes                    
    def add(self, position:glm.vec3|list=None, scale:glm.vec3|list=None, rotation:glm.vec3|list=None, nodes:list=None, model=None, collider=None, physics_body=None, name:str='node', camera=None):
        """
        Adds a node to the top level array and returns it. 
        """
        node = Node(self, position, scale, rotation, nodes, model, collider, physics_body, name, camera)
        node.init_physics_body()
        node.update_parents()
        self.nodes.append(node)
        return node
    
    # # just create
    def create(self, position:glm.vec3|list=None, scale:glm.vec3|list=None, rotation:glm.vec3|list=None, nodes:list=None, model=None, collider=None, physics_body=None, name:str='node', camera=None):
        """
        Only creates a collectino and returns it, does not add to top level array. 
        """
        return Node(self, position, scale, rotation, nodes, model, collider, physics_body, name, camera)