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

    def update(self) -> None:
        """
        Updates the physics and in the scene
        """
        
        self.camera.update()
        self.node_handler.update()

    def render(self) -> None:
        """
        Renders all the nodes with meshes in the scene
        """

        self.shader_handler.write()
        self.node_handler.render()
        self.draw_handler.render()
    
    def add_node(self, 
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
            static:              bool=True):
        
        if material: self.material_handler.add(material)
        else: material = self.material_handler.base

        return self.node_handler.add(position, scale, rotation, forward, mesh, material, velocity, rotational_velocity, physics, mass, collisions, collider, static_friction, kinetic_friction, elasticity, collision_group, name, tags, static)


    def set_engine(self, engine: any) -> None:
        """
        Sets the back references to the engine and creates handlers with the context
        """

        self.engine = engine
        self.ctx    = engine.ctx

        self.camera           = FreeCamera()
        self.shader_handler   = ShaderHandler(self)
        self.physics_engine   = PhysicsEngine()
        self.node_handler     = NodeHandler(self)
        self.collider_handler = ColliderHandler(self)
        self.material_handler = MaterialHandler(self)
        self.light_handler    = LightHandler(self)
        self.draw_handler     = DrawHandler(self)

    @property
    def camera(self): return self._camera

    @camera.setter
    def camera(self, value: Camera):
        if not value: return
        if not isinstance(value, Camera):
            raise TypeError(f'Scene: Invalid camera type: {type(value)}. Expected type bsk.Camera')
        self._camera = value
        self._camera.scene = self