from scripts.camera import *
from scripts.collisions.collider_handler import ColliderHandler
from scripts.model_handler import ModelHandler
from scripts.nodes.node_handler import NodeHandler
from scripts.physics.physics_body_handler import PhysicsBodyHandler
from scripts.render.material_handler import MaterialHandler
from scripts.render.light_handler import LightHandler
from scripts.render.sky import Sky
from scripts.skeletons.skeleton_handler import SkeletonHandler
from scripts.skeletons.joints import * 
from scripts.file_manager.load_scene import load_scene
from scripts.skeletons.animation import *
from scripts.skeletons.joints import *
from scripts.render.vbo_handler import CubeVBO, RuntimeVBO
from random import randint, uniform
from scripts.generic.math_functions import get_model_matrix
import moderngl as mgl

class Scene:
    def __init__(self, engine, project, editor=False) -> None:
        """
        Contains all data for scene
        """

        # Stores the engine, project, and ctx
        self.engine = engine
        self.project = project
        self.ctx = self.engine.ctx

        # Gets handlers from parent project
        self.vao_handler = self.project.vao_handler
        # model handler
        self.sky = Sky(self)
        self.material_handler = MaterialHandler(self)
        self.model_handler = ModelHandler(self)
        self.collider_handler = ColliderHandler(self)
        self.physics_body_handler = PhysicsBodyHandler(self)
        self.node_handler = NodeHandler(self)
        self.skeleton_handler = SkeletonHandler(self)
        self.light_handler = LightHandler(self)
        
        # Makes a free cam
        self.camera = FollowCamera(self.engine, radius = 40, scene=self)
                
        load_scene(self, "room4")
        if not editor: 
            self.load_user_scripts()
            self.collider_handler.construct_bvh()
                
    def use(self, camera=True):
        """
        Updates project handlers to use this scene
        """

        self.vao_handler.shader_handler.set_camera(self.camera)
        if camera: self.camera.use()
        self.vao_handler.generate_framebuffer()
        self.vao_handler.shader_handler.write_all_uniforms()
        self.project.texture_handler.write_textures()
        self.project.texture_handler.write_textures('batch')
        self.light_handler.write('batch')
        self.material_handler.write('batch')

    def update(self, camera=True):
        """
        Updates uniforms, and camera
        """

        self.model_handler.update()
        self.vao_handler.shader_handler.update_uniforms()
        if camera: self.camera.update()
        if camera and self.on_frame: exec(self.on_frame)

    def render(self, display=True):
        """
        Redners all instances
        """

        self.ctx.disable(flags=mgl.CULL_FACE)
        self.vao_handler.framebuffer.clear()
        self.vao_handler.framebuffer.use()
        self.sky.render()
        self.model_handler.render()

        if not display: return

        self.ctx.screen.use()
        self.vao_handler.shader_handler.programs['frame']['screenTexture'] = 0
        self.vao_handler.frame_texture.use(location=0)
        self.vao_handler.vaos['frame'].render()

    def release(self):
        """
        Releases scene's VAOs
        """

        self.vao_handler.release()
    
    def load_user_scripts(self):
        # Load init scripts
        with open(f'user_scripts/scene_on_init.py') as file: scene_on_init = compile(file.read(), 'scene_on_init', 'exec')
        with open(f'user_scripts/add_john.py')      as file: add_john          = compile(file.read(), 'add_john', 'exec')
        # Load runtime scripts
        with open(f'user_scripts/bottom_on_frame.py')   as file: bottom_on_frame   = compile(file.read(), 'bottom_on_frame', 'exec')
        with open(f'user_scripts/bottom_on_frame.py')   as file: bottom_on_init    = compile(file.read(), 'bottom_on_init', 'exec')
        with open(f'user_scripts/face_camera.py')       as file: face_camera       = compile(file.read(), 'face_camera', 'exec')
        with open(f'user_scripts/walking_animation.py') as file: walking_animation = compile(file.read(), 'walking_animation', 'exec')
        with open(f'user_scripts/scene_on_frame.py')    as file: scene_on_frame    = compile(file.read(), 'scene_on_frame', 'exec')
        with open(f'user_scripts/head_on_frame.py')     as file: head_on_frame     = compile(file.read(), 'head_on_frame', 'exec')
        # Exec on init scripts
        exec(scene_on_init)
        exec(add_john)
        
        self.on_tick = None # TODO add functionality
        self.on_frame = scene_on_frame

    def get_model_node_at(self, x:int, y:int, distance:float=1e5, has_collider:bool=False, has_physics_body:bool=False, material:str=None, tags:list[str]=''):
        """
        Gets the closest node at the pixel position
        """
        best_node  = None
        pixel_position = glm.vec2(x, y)
        
        # determine forward vector from screen click
        inv_proj, inv_view = glm.inverse(self.camera.m_proj), glm.inverse(self.camera.m_view)
        ndc   = glm.vec4(2 * pixel_position[0] / self.project.engine.win_size[0] - 1, 1 - 2 * pixel_position[1] / self.project.engine.win_size[1], 1, 1)
        point = inv_proj * ndc
        point /= point.w
        forward = glm.normalize(glm.vec3(inv_view * glm.vec4(point.x, point.y, point.z, 0)))
        
        best_node, point = self.camera.get_model_node_at(forward = forward, max_distance = distance, has_collider = has_collider, has_physics_body = has_physics_body, material = material, tags = tags)
                
        return best_node