import moderngl as mgl
import glm
from .render.shader_handler import ShaderHandler
from .mesh.mesh import Mesh
from .render.material import Material
from .render.material_handler import MaterialHandler
from .render.light_handler import LightHandler
from .render.camera import Camera, FreeCamera
from .nodes.node_handler import NodeHandler
from .physics.physics_engine import PhysicsEngine
from .collisions.collider_handler import ColliderHandler
from .draw.draw_handler import DrawHandler
from .render.sky import Sky
from .render.frame import Frame
from .render.shader import Shader
from .nodes.node import Node
from .generic.collisions import moller_trumbore

class Scene():
    engine: any
    """Parent engine of the scene"""
    ctx: mgl.Context
    """Reference to the engine context"""

    def __init__(self) -> None:
        """
        Basilisk scene object. Contains all nodes for the scene
        """

        self.engine = None
        self.ctx    = None

        self.camera           = None
        self.shader_handler   = None
        self.node_handler     = None
        self.material_handler = None
        self.light_handler    = None
        self.draw_handler     = None
        self.sky              = None
        self.frame            = None

    def update(self) -> None:
        """
        Updates the physics and in the scene
        """
        
        self.node_handler.update()
        self.camera.update()
        self.collider_handler.resolve_collisions()

    def render(self) -> None:
        """
        Renders all the nodes with meshes in the scene
        """

        self.frame.use()
        self.shader_handler.write()
        if self.sky: self.sky.render()
        self.node_handler.render()
        self.draw_handler.render()

        if self.engine.headless: return
        self.frame.render()
    
    def add(self, bsk_object: Node):
        """
        Adds an object to the scene. Can pass in any scene objects:
        Argument overloads:
            object: Node - Adds the given node to the scene.
        """
        
        if isinstance(bsk_object, Node):
            # Add a node to the scene
            return self.node_handler.add(bsk_object)
        # Light

        # Mesh

        # Sky

        # Scene

    def remove_node(self, node):
        """
        Removes the given node from the scene
        """

        self.node_handler.remove(node)

    def set_engine(self, engine: any) -> None:
        """
        Sets the back references to the engine and creates handlers with the context
        """

        self.engine = engine
        self.ctx    = engine.ctx

        self.camera           = FreeCamera()
        self.shader_handler   = ShaderHandler(self)
        self.material_handler = MaterialHandler(self)
        self.light_handler    = LightHandler(self)
        self.physics_engine   = PhysicsEngine()
        self.node_handler     = NodeHandler(self)
        self.collider_handler = ColliderHandler(self)
        self.draw_handler     = DrawHandler(self)
        self.frame            = Frame(self)
        self.sky              = Sky(self.engine)
        
    def raycast(self, position: glm.vec3=None, forward: glm.vec3=None, max_distance: float=1e5, has_collisions: bool=None, has_physics: bool=None, tags: list[str]=[]) -> tuple[Node, glm.vec3]:
        """
        Ray cast from any posiiton and forward vector and returns the nearest node. If no position or forward is given, uses the scene camera's current position and forward
        """
        if not position: position = self.camera.position
        if not forward: forward = self.camera.forward
        forward = glm.normalize(forward)
        
        # if we are filtering for collisions, use the broad BVH to improve performance
        if has_collisions: 
            colliders = self.collider_handler.bvh.get_line_collided(position, forward)
            nodes = [collider.node for collider in colliders]
            
            def is_valid(node: Node) -> bool:
                return all([
                    has_collisions is None or bool(node.collider) == has_collisions,
                    has_physics is None or bool(node.physics_body) == has_physics,
                    all(tag in node.tags for tag in tags)
                ])
                
            nodes: list[Node] = list(filter(lambda node: is_valid(node), nodes))
        
        # if we are not filtering for collisions, filter nodes and 
        else: nodes = self.node_handler.get_all(collisions=has_collisions, physics=has_physics, tags=tags)

        # determine closest node
        best_distance, best_point, best_node = max_distance, None, None
        position_two = position + forward
        for node in nodes:
            
            inv_mat = glm.inverse(node.model_matrix)
            relative_position = inv_mat * position
            relative_forward = glm.normalize(inv_mat * position_two - relative_position)
            
            for triangle in node.mesh.indices:
                intersection = moller_trumbore(relative_position, relative_forward, [node.mesh.points[i] for i in triangle])
                if not intersection: continue
                intersection = node.model_matrix * intersection
                distance = glm.length(intersection - position)
                if distance < best_distance:
                    best_distance = distance
                    best_point    = intersection
                    best_node     = node
                    
        return best_node, best_point
    
    def raycast_mouse(self, position: tuple[int, int] | glm.vec2, max_distance: float=1e5, has_collisions: bool=None, has_pshyics: bool=None, tags: list[str]=[]) -> tuple[Node, glm.vec3]:
        """
        Ray casts from the mouse position with respect to the camera. Returns the nearest node that was clicked, if none was clicked, returns None. 
        """
        # derive forward vector from mouse click position
        position = glm.vec2(position)
        inv_proj, inv_view = glm.inverse(self.camera.m_proj), glm.inverse(self.camera.m_view)
        ndc   = glm.vec4(2 * position[0] / self.engine.win_size[0] - 1, 1 - 2 * position[1] / self.engine.win_size[1], 1, 1)
        point = inv_proj * ndc
        point /= point.w
        forward = glm.normalize(glm.vec3(inv_view * glm.vec4(point.x, point.y, point.z, 0)))
        
        return self.raycast(
            position=self.camera.position,
            forward=forward,
            max_distance=max_distance,
            has_collisions=has_collisions,
            has_physics=has_pshyics,
            tags=tags
        )

    @property
    def camera(self): return self._camera
    @property
    def sky(self): return self._sky

    @camera.setter
    def camera(self, value: Camera):
        if not value: return
        if not isinstance(value, Camera):
            raise TypeError(f'Scene: Invalid camera type: {type(value)}. Expected type bsk.Camera')
        self._camera = value
        self._camera.scene = self

    @sky.setter
    def sky(self, value: Sky):
        if not isinstance(value, Sky) and not isinstance(value, type(None)):
            raise TypeError(f'Scene: Invalid sky type: {type(value)}. Expected type bsk.Sky or None')
        self._sky = value
        if value: self._sky.write()