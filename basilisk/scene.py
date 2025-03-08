import moderngl as mgl
import glm
import pygame as pg

from .mesh.mesh import Mesh
from .render.material import Material
from .render.shader import Shader
from .render.light_handler import LightHandler
from .render.camera import Camera, FreeCamera
from .nodes.node_handler import NodeHandler
from .physics.physics_engine import PhysicsEngine
from .collisions.collider_handler import ColliderHandler
from .render.sky import Sky
from .render.frame import Frame
from .particles.particle_handler import ParticleHandler
from .nodes.node import Node
from .generic.collisions import moller_trumbore
from .generic.raycast_result import RaycastResult
from .render.post_process import PostProcess
from .render.framebuffer import Framebuffer

class Scene():
    engine: ...=None
    """Parent engine of the scene"""
    ctx: mgl.Context
    """Reference to the engine context"""
    camera: Camera=None
    """"""
    light_handler: LightHandler=None
    """"""
    physics_engine: PhysicsEngine=None
    """"""
    node_handler: NodeHandler=None
    """"""

    def __init__(self, engine: ..., shader: Shader=None) -> None:
        """
        Basilisk scene object. Contains all nodes for the scene
        """

        self.engine = engine
        self.ctx    = engine.ctx
        self.shader = shader if shader else engine.shader
        self.camera           = FreeCamera()
        self.light_handler    = LightHandler(self)
        self.physics_engine   = PhysicsEngine()
        self.node_handler     = NodeHandler(self)
        self.particle         = ParticleHandler(self)
        self.collider_handler = ColliderHandler(self)
        self.sky              = Sky(self.engine)


    def update(self, render: bool=True, nodes: bool=True, particles: bool=True, collisions: bool=True) -> None:
        """
        Updates the physics and in the scene
        """
        
        # Check that the engine is still running
        self.engine._update()
        if not self.engine.running: return

        # Update based on the given parameters
        if nodes: self.node_handler.update()
        if particles: self.particle.update()
        if self.engine.event_resize: self.camera.use()
        self.camera.update()
        
        if collisions and self.engine.delta_time < 0.5: # TODO this will cause physics to slow down when on low frame rate, this is probabl;y acceptable
            self.collider_handler.resolve_collisions()

        # Render by default to the engine frame
        if not render: return

        # Check if the user is giving a destination
        if not isinstance(render, bool): self.render(renders)
        else: self.render()

    def render(self, render_target: Framebuffer|Frame=None) -> None:
        """
        Renders all the nodes with meshes in the scene
        """

        if render_target:
            show = False
        else:
            render_target = self.engine.frame
            show = True

        render_target.use()
        self.engine.shader_handler.write(self)
        if self.sky: self.sky.render()
        self.node_handler.render()
        self.particle.render()


        if self.engine.headless or not show: return


    def add(self, *objects: Node | None) -> None | Node | list:
        """
        Adds the given object(s) to the scene. Can pass in any scene objects:
        Argument overloads:
            object: Node - Adds the given node to the scene.
        """
        
        # List of all return values for the added objects
        returns = []

        # Loop through all objects passed in
        for bsk_object in objects:

            # Considered well defined behavior to add None
            if isinstance(bsk_object, type(None)):
                continue

            # Add a node to the scene
            elif isinstance(bsk_object, Node):
                returns.append(self.node_handler.add(bsk_object)); continue
            
            # Add a node to the scene
            elif isinstance(bsk_object, PostProcess):
                returns.append(self.engine.frame.add_post_process(bsk_object)); continue
            
            
            # Recived incompatable type
            else:
                raise ValueError(f'scene.add: Incompatable object add type {type(bsk_object)}')

        # Return based on what the user passed in
        if not returns: return None
        if len(returns) == 1: return returns[0]
        return returns

    def remove(self, *objects: Node | None) -> None | Node | list:
        """
        Removes the given baskilsk object from the scene
        """

        # List of all return values for the added objects
        returns = []

        # Loop through all objects passed in
        for bsk_object in objects:

            # Considered well defined behavior to remove None
            if isinstance(bsk_object, type(None)):
                continue

            # Remove a node from the scene
            elif isinstance(bsk_object, Node):
                returns.append(self.node_handler.remove(bsk_object)); continue

            # Recived incompatable type
            else:
                raise ValueError(f'scene.remove: Incompatable object remove type {type(bsk_object)}')

        # Return based on what the user passed in
        if not returns: return None
        if len(returns) == 1: return returns[0]
        return returns

    def set_engine(self, engine: any) -> None:
        """
        Sets the back references to the engine and creates handlers with the context
        """

        if not self.engine: 
            self.engine = engine
            self.ctx    = engine.ctx
            self.init_handlers()
        else:
            self.engine = engine
            self.ctx    = engine.ctx
        
    def raycast(self, position: glm.vec3=None, forward: glm.vec3=None, max_distance: float=1e5, has_collisions: bool=None, has_physics: bool=None, tags: list[str]=[]) -> RaycastResult:
        """
        Ray cast from any posiiton and forward vector and returns a RaycastResult eith the nearest node. 
        If no position or forward is given, uses the scene camera's current position and forward
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
        best_distance, best_point, best_node, best_triangle = max_distance, None, None, None
        position_two = position + forward
        for node in nodes:
            
            inv_mat = glm.inverse(node.model_matrix)
            relative_position = inv_mat * position
            relative_forward = glm.normalize(inv_mat * position_two - relative_position)
            
            triangles = [node.mesh.indices[i] for i in node.mesh.get_line_collided(relative_position, relative_forward)]
            
            for triangle in triangles:
                intersection = moller_trumbore(relative_position, relative_forward, [node.mesh.points[i] for i in triangle])
                if not intersection: continue
                intersection = node.model_matrix * intersection
                distance = glm.length(intersection - position)
                if distance < best_distance:
                    best_distance = distance
                    best_point    = intersection
                    best_node     = node
                    best_triangle = triangle
        
        if not best_node: return RaycastResult(best_node, best_point, None)
        
        points = [best_node.model_matrix * best_node.mesh.points[t] for t in best_triangle]
        normal = glm.normalize(glm.cross(points[1] - points[0], points[2] - points[0]))
                    
        return RaycastResult(best_node, best_point, normal)
    
    def raycast_mouse(self, position: tuple[int, int] | glm.vec2, max_distance: float=1e5, has_collisions: bool=None, has_pshyics: bool=None, tags: list[str]=[]) -> RaycastResult:
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
        
    def get(self, position: glm.vec3=None, scale: glm.vec3=None, rotation: glm.quat=None, forward: glm.vec3=None, mesh: Mesh=None, material: Material=None, velocity: glm.vec3=None, rotational_velocity: glm.quat=None, physics: bool=None, mass: float=None, collisions: bool=None, static_friction: float=None, kinetic_friction: float=None, elasticity: float=None, collision_group: float=None, name: str=None, tags: list[str]=None,static: bool=None) -> Node:
        """
        Returns the first node with the given traits
        """
        self.node_handler.get(position, scale, rotation, forward, mesh, material, velocity, rotational_velocity, physics, mass, collisions, static_friction, kinetic_friction, elasticity, collision_group, name, tags, static)
    
    def get_all(self, position: glm.vec3=None, scale: glm.vec3=None, rotation: glm.quat=None, forward: glm.vec3=None, mesh: Mesh=None, material: Material=None, velocity: glm.vec3=None, rotational_velocity: glm.quat=None, physics: bool=None, mass: float=None, collisions: bool=None, static_friction: float=None, kinetic_friction: float=None, elasticity: float=None, collision_group: float=None, name: str=None, tags: list[str]=None,static: bool=None) -> list[Node]:
        """
        Returns all nodes with the given traits
        """
        self.node_handler.get_all(position, scale, rotation, forward, mesh, material, velocity, rotational_velocity, physics, mass, collisions, static_friction, kinetic_friction, elasticity, collision_group, name, tags, static)

    @property
    def camera(self): return self._camera
    @property
    def sky(self): return self._sky
    @property
    def nodes(self): return self.node_handler.nodes
    @property
    def shader(self): return self._shader

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

    @shader.setter
    def shader(self, value):
        self._shader = value
        value.set_main(self)