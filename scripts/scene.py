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
from scripts.file_manager.save_scene import save_scene
from scripts.file_manager.load_scene import load_scene
from scripts.file_manager.get_file import save_file_selector, load_file_selector
from scripts.skeletons.animation import *
from scripts.skeletons.joints import *
from random import randint, uniform
from scripts.generic.math_functions import get_model_matrix
import moderngl as mgl
import platform

class Scene:
    def __init__(self, engine, project) -> None:
        """
        Contains all data for scene
        """

        # Stores the engine, project, and ctx
        self.engine = engine
        self.project = project
        self.ctx = self.engine.ctx

        # Makes a free cam
        self.camera = FollowCamera(self.engine, radius = 40)

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
                
        with open(f'user_scripts/scene_on_init.py') as file: scene_on_init = compile(file.read(), 'scene_on_init', 'exec')
        exec(scene_on_init)
        
        with open(f'user_scripts/bottom_on_frame.py')   as file: bottom_on_frame   = compile(file.read(), 'bottom_on_frame', 'exec')
        with open(f'user_scripts/face_camera.py')       as file: face_camera       = compile(file.read(), 'face_camera', 'exec')
        with open(f'user_scripts/walking_animation.py') as file: walking_animation = compile(file.read(), 'walking_animation', 'exec')
        with open(f'user_scripts/scene_on_frame.py')    as file: scene_on_frame    = compile(file.read(), 'scene_on_frame', 'exec')
        with open(f'user_scripts/head_on_frame.py')     as file: head_on_frame     = compile(file.read(), 'head_on_frame', 'exec')
        
        self.on_tick = None # TODO add functionality
        self.on_frame = scene_on_frame
        load_scene(self, "base_save")
        
        #save_scene(self, "test_save")
            
        # # level 1 #################################################################################################
        
        # self.node_handler.add(
        #     position=(-30, -4, 40),
        #     scale=(10, 1, 80),
        #     model='cube', 
        #     material='brick',
        #     collider=self.collider_handler.add(vbo='cube', static=True),
        #     physics_body=None,
        #     name='floor'
        # )
        
        # self.node_handler.add(
        #     position=(30, -4, 40),
        #     scale=(10, 1, 80),
        #     model='cube', 
        #     material='brick',
        #     collider=self.collider_handler.add(vbo='cube', static=True),
        #     physics_body=None,
        #     name='floor'
        # )
        
        # self.node_handler.add(
        #     position=(0, -4, 40),
        #     scale=(20, 1, 80),
        #     model='cube', 
        #     material='red_pink',
        #     collider=self.collider_handler.add(vbo='cube', static=True),
        #     physics_body=None,
        #     name='floor'
        # )
        
        # self.node_handler.add(
        #     position=(9, 10, 80),
        #     scale=(1, 15, 4),
        #     rotation=(0, 0, 0),
        #     nodes=[],
        #     model='cube', 
        #     material='brick',
        #     collider=self.collider_handler.add(vbo='cube', static=True),
        #     physics_body=None,
        #     name='clamp'
        # )
        
        # self.node_handler.add(
        #     position=(25, 12.1, 85),
        #     scale=(15, 15, 1),
        #     model='cube', 
        #     material='brick',
        #     collider=self.collider_handler.add(vbo='cube', static=True),
        #     physics_body=None,
        #     name='clamp front'
        # )
        
        # self.node_handler.add(
        #     position=(8, 10, 73),
        #     scale=(3, 15, 3),
        #     rotation=(0, 0, 0),
        #     nodes=[],
        #     model='cube', 
        #     material='brick',
        #     collider=self.collider_handler.add(vbo='cube', static=True),
        #     physics_body=None,
        #     name='clamp back'
        # )
        
        # self.node_handler.add(
        #     position=(-9, 10, 80),
        #     scale=(1, 15, 4),
        #     rotation=(0, 0, 0),
        #     nodes=[],
        #     model='cube', 
        #     material='brick',
        #     collider=self.collider_handler.add(vbo='cube', static=True),
        #     physics_body=None,
        #     name='clamp'
        # )
        
        # self.node_handler.add(
        #     position=(-25, 12.1, 85),
        #     scale=(15, 15, 1),
        #     rotation=(0, 0, 0),
        #     nodes=[],
        #     model='cube', 
        #     material='brick',
        #     collider=self.collider_handler.add(vbo='cube', static=True),
        #     physics_body=None,
        #     name='clamp front'
        # )
        
        # self.node_handler.add(
        #     position=(-8, 10, 73),
        #     scale=(3, 15, 3),
        #     rotation=(0, 0, 0),
        #     nodes=[],
        #     model='cube', 
        #     material='brick',
        #     collider=self.collider_handler.add(vbo='cube', static=True),
        #     physics_body=None,
        #     name='clamp back'
        # )
        
        # # self.node_handler.add(
        # #     position=(0, 2, 80),
        # #     scale=(8, 10, 3),
        # #     model='cube',
        # #     material='yellow',
        # #     collider=self.collider_handler.add(vbo='cube', static=False),
        # #     physics_body=self.physics_body_handler.add(mass=1000),
        # #     name='door'
        # # )
        
        # # level 2 #################################################################################################
        
        # self.node_handler.add(
        #     position=(0, 7, -35),
        #     scale=(40, 10, 5),
        #     rotation=(0, 0, 0),
        #     nodes=[],
        #     model='cube', 
        #     material='brick',
        #     collider=self.collider_handler.add(vbo='cube', static=True),
        #     physics_body=None,
        #     name='platform'
        # )
        
        # self.node_handler.add(
        #     position=(0, 7, 0),
        #     scale=(10, 10, 10),
        #     model='cube',
        #     material='yellow',
        #     collider=self.collider_handler.add(vbo='cube', static=False),
        #     physics_body=self.physics_body_handler.add(mass=100),
        #     name='box'
        # )
        
        # # level 3 ################################################################################################## (0, +20, -80)
        
        # self.node_handler.add(
        #     position=(0, 16, -80),
        #     scale=(40, 1, 40),
        #     rotation=(0, 0, 0),
        #     nodes=[],
        #     model='cube', 
        #     material='brick',
        #     collider=self.collider_handler.add(vbo='cube', static=True),
        #     physics_body=None,
        #     name='floor'
        # )
        
        # self.node_handler.add(
        #     position=(0, 27, -115),
        #     scale=(40, 10, 5),
        #     rotation=(0, 0, 0),
        #     nodes=[],
        #     model='cube', 
        #     material='brick',
        #     collider=self.collider_handler.add(vbo='cube', static=True),
        #     physics_body=None,
        #     name='platform'
        # )
        
        # self.node_handler.add(
        #     position=(0, 27, -80),
        #     scale=(10, 10, 10),
        #     model='cube',
        #     material='yellow',
        #     collider=self.collider_handler.add(vbo='cube', static=False),
        #     physics_body=self.physics_body_handler.add(mass=100),
        #     name='box'
        # )
        
        # # john bitcock #############################################################################################
        
        # cock_pos = glm.vec3(0, -2, 110)
        
        # left_foot=self.node_handler.add(
        #     position=cock_pos + glm.vec3(0.5, 0.25, 0),
        #     scale=(0.3, 0.25, 0.3),
        #     rotation=(0, 0, 0),
        #     model='cube', 
        #     material='white',
        #     physics_body=self.physics_body_handler.add(mass=10),
        #     name='left foot',
            
        #     nodes=[
        #         # leg puff
        #         self.node_handler.create(
        #             position=(0, -0.6, 0),
        #             scale=(1.1, 0.05, 1.1),
        #             model='cube', 
        #             material='baby_blue',
        #             name='leg puff'
        #         ),
        #         self.node_handler.create(
        #             position=(0, -0.5, 0),
        #             scale=(1.1, 0.05, 1.1),
        #             model='cube', 
        #             material='yellow',
        #             name='leg puff'
        #         ),
        #         self.node_handler.create(
        #             position=(0, -0.4, 0),
        #             scale=(1.1, 0.05, 1.1),
        #             model='cube', 
        #             material='baby_blue',
        #             name='leg puff'
        #         )
        #     ]
        # )
        
        # left_foot.on_frame=face_camera
        
        # left_knee=self.node_handler.add(
        #     position=cock_pos + glm.vec3(0.5, 0.75, 0),
        #     scale=(0.3, 0.7, 0.3),
        #     rotation=(0, 0, 0),
        #     model='cube', 
        #     material='white',
        #     physics_body=self.physics_body_handler.add(mass=20),
        #     name='left foot',
        # )
        
        # right_foot=self.node_handler.add(
        #     position=cock_pos + glm.vec3(-0.5, 0.25, 0),
        #     scale=(0.3, 0.25, 0.3),
        #     rotation=(0, 0, 0),
        #     model='cube',
        #     material='white',
        #     physics_body=self.physics_body_handler.add(mass=10),
        #     name='right foot',
            
        #     nodes=[
        #         # leg puff
        #         self.node_handler.create(
        #             position=(0, -0.6, 0),
        #             scale=(1.1, 0.05, 1.1),
        #             model='cube', 
        #             material='baby_blue',
        #             name='leg puff'
        #         ),
        #         self.node_handler.create(
        #             position=(0, -0.5, 0),
        #             scale=(1.1, 0.05, 1.1),
        #             model='cube', 
        #             material='yellow',
        #             name='leg puff'
        #         ),
        #         self.node_handler.create(
        #             position=(0, -0.4, 0),
        #             scale=(1.1, 0.05, 1.1),
        #             model='cube',
        #             material='baby_blue',
        #             name='leg puff'
        #         )
        #     ]
        # )
        
        # right_foot.on_frame=face_camera
        
        # right_knee=self.node_handler.add(
        #     position=cock_pos + glm.vec3(-0.5, 0.75, 0),
        #     scale=(0.3, 0.7, 0.3),
        #     rotation=(0, 0, 0),
        #     model='cube', 
        #     material='white',
        #     physics_body=self.physics_body_handler.add(mass=20),
        #     name='left foot',
        # )

        # bottom=self.node_handler.add(
        #     position=cock_pos + glm.vec3(0, 0.5, 0),
        #     scale=(1, 1.5, 1),
        #     rotation=(0, 0, 0),
        #     collider=self.collider_handler.add(vbo='cube', static=False, group='john'),
        #     physics_body=self.physics_body_handler.add(mass=2000),
        #     nodes=[
        #         self.node_handler.create(
        #             position=(-2, 6, -2),
        #             camera=self.camera
        #         ),
        #         self.node_handler.create(
        #             position=glm.vec3(0, 0.5, 0),
        #             scale=(1, 0.25, 1),
        #             rotation=(0, 0, 0),
        #             model='cube', 
        #             material='white',
        #         )
        #     ],
        #     name='bottom'
        # )
        
        # bottom.on_frame = bottom_on_frame
        
        # middle=self.node_handler.add(
        #     position=cock_pos + glm.vec3(0, 2.5, 0),
        #     scale=(1, 0.5, 1),
        #     rotation=(0, 0, 0),
        #     model='cube', 
        #     material='white',
        #     collider=self.collider_handler.add(vbo='cube', static=False, group='john'),
        #     physics_body=self.physics_body_handler.add(mass=20),
        #     name='middle',
            
        #     nodes=[
        #         self.node_handler.create(
        #             position=glm.vec3(0, 0, 1),
        #             scale=(0.8, 0.8, 0.01),
        #             rotation=(0, 0, 0),
        #             model='cube', 
        #             material='grey',
        #         ),
        #         self.node_handler.create(
        #             position=glm.vec3(0, 0.1, 1),
        #             scale=(0.6, 0.6, 0.015),
        #             rotation=(0, 0, 0),
        #             model='cube', 
        #             material='white',
        #         ),
        #         self.node_handler.create(
        #             position=glm.vec3(0, 0.1, 1),
        #             scale=(0.6, 0.3, 0.02),
        #             rotation=(0, 0, 0),
        #             model='cube', 
        #             material='baby_blue',
        #         ),
        #         self.node_handler.create(
        #             position=glm.vec3(0, 0.1, 1),
        #             scale=(0.35, 0.15, 0.025),
        #             rotation=(0, 0, 0),
        #             model='cube', 
        #             material='black',
        #         ),
        #         self.node_handler.create(
        #             position=(0.2, 0.1, 1.02),
        #             scale=(0.08, 0.03, 0.08),
        #             rotation=(glm.pi()/2, 0, 0),
        #             model='cylinder', 
        #             material='white',
        #         ),
        #         self.node_handler.create(
        #             position=(-0.2, 0.1, 1.02),
        #             scale=(0.08, 0.03, 0.08),
        #             rotation=(glm.pi()/2, 0, 0),
        #             model='cylinder', 
        #             material='white',
        #         ),
        #     ]
        # )
        
        # middle.on_frame = face_camera
        
        # top=self.node_handler.add(
        #     position=cock_pos + glm.vec3(0, 3.5, 0),
        #     scale=(1, 0.5, 1),
        #     rotation=(0, 0, 0),
        #     model='cube', 
        #     material='baby_blue',
        #     collider=self.collider_handler.add(vbo='cube', static=False, group='john'),
        #     physics_body=self.physics_body_handler.add(mass=20),
        #     name='top',
            
        #     nodes=[
        #         # id
        #         self.node_handler.create(
        #             position=glm.vec3(0.5, 0.2, 1),
        #             scale=(0.3, 0.1, 0.05),
        #             rotation=(0, 0, -0.1),
        #             model='cube', 
        #             material='red_pink',
        #         ),
        #         self.node_handler.create(
        #             position=glm.vec3(0.51, 0, 1),
        #             scale=(0.3, 0.1, 0.05),
        #             rotation=(0, 0, -0.1),
        #             model='cube', 
        #             material='white',
        #         ),
        #         self.node_handler.create(
        #             position=glm.vec3(0.52, -0.2, 1),
        #             scale=(0.3, 0.1, 0.05),
        #             rotation=(0, 0, -0.1),
        #             model='cube', 
        #             material='red_pink',
        #         ),
        #         self.node_handler.create(
        #             position=glm.vec3(0.5, 0.15, 1),
        #             scale=(0.1, 0.03, 0.055),
        #             rotation=(0, 0, 0.1),
        #             model='cube', 
        #             material='black',
        #         ),
        #         self.node_handler.create(
        #             position=glm.vec3(0.5, -0.05, 1),
        #             scale=(0.03, 0.2, 0.055),
        #             rotation=(0, 0, 0.1),
        #             model='cube', 
        #             material='black',
        #         ),
        #         self.node_handler.create(
        #             position=glm.vec3(0.425, -0.15, 1),
        #             scale=(0.05, 0.03, 0.055),
        #             rotation=(0, 0, 0.6),
        #             model='cube', 
        #             material='black',
        #         ),
        #         self.node_handler.create(
        #             position=glm.vec3(0.5, 0.25, 1.05),
        #             scale=(0.05, 0.1, 0.05),
        #             rotation=(glm.pi()/2, 0, 0),
        #             model='cylinder', 
        #             material='grey',
        #         ),
        #         # buttons
                
        #     ]
        # )
        
        # top.on_frame = face_camera
        
        # left_arm=self.node_handler.add(
        #     position=cock_pos + glm.vec3(1.3, 2.5, 0),
        #     scale=(0.3, 1.25, 0.3),
        #     rotation=(0, 0, 0),
        #     model='cube',
        #     material='white',
        #     collider=self.collider_handler.add(vbo='cube', static=False),
        #     physics_body=self.physics_body_handler.add(mass=20),
        #     name='left arm',
            
        #     nodes=[
        #         # arm puff
        #         self.node_handler.create(
        #             position=(0, -0.64, 0),
        #             scale=(1.1, 0.03, 1.1),
        #             model='cube', 
        #             material='baby_blue',
        #             name='arm puff'
        #         ),
        #         self.node_handler.create(
        #             position=(0, -0.7, 0),
        #             scale=(1.1, 0.03, 1.1),
        #             model='cube', 
        #             material='yellow',
        #             name='arm puff'
        #         ),
        #         self.node_handler.create(
        #             position=(0, -0.76, 0),
        #             scale=(1.1, 0.03, 1.1),
        #             model='cube', 
        #             material='baby_blue',
        #             name='arm puff'
        #         ),
        #     ]
        # )
        
        # right_arm=self.node_handler.add(
        #     position=cock_pos + glm.vec3(-1.3, 2.5, 0),
        #     scale=(0.3, 1.25, 0.3),
        #     rotation=(0, 0, 0),
        #     model='cube',
        #     material='white',
        #     collider=self.collider_handler.add(vbo='cube', static=False),
        #     physics_body=self.physics_body_handler.add(mass=20),
        #     name='right arm',
            
        #     nodes=[
        #         # arm puff
        #         self.node_handler.create(
        #             position=(0, -0.64, 0),
        #             scale=(1.1, 0.03, 1.1),
        #             model='cube', 
        #             material='baby_blue',
        #             name='arm puff'
        #         ),
        #         self.node_handler.create(
        #             position=(0, -0.7, 0),
        #             scale=(1.1, 0.03, 1.1),
        #             model='cube', 
        #             material='yellow',
        #             name='arm puff'
        #         ),
        #         self.node_handler.create(
        #             position=(0, -0.76, 0),
        #             scale=(1.1, 0.03, 1.1),
        #             model='cube', 
        #             material='baby_blue',
        #             name='arm puff'
        #         ),
        #     ]
        # )
        
        # head=self.node_handler.add(
        #     position=cock_pos + glm.vec3(0, 4.7, 0),
        #     scale=(0.7, 0.7, 0.7),
        #     rotation=(0, 0, 0),
        #     model='cube', 
        #     material='white',
        #     collider=self.collider_handler.add(vbo='cube', static=False, group='john'),
        #     physics_body=self.physics_body_handler.add(mass=20),
        #     name='head',
            
        #     nodes=[
        #         # cutter
        #         self.node_handler.create(
        #             position=(1, 0, 0),
        #             scale=(0.2, 0.5, 0.5),
        #             model='cube', 
        #             material='grey',
        #             name='cutter base'
        #         ),
        #         self.node_handler.create(
        #             position=(1.1, 1, 0),
        #             scale=(0.05, 0.5, 0.05),
        #             model='cube', 
        #             material='dark_grey',
        #             name='cutter antenna'
        #         ),
        #         self.node_handler.create(
        #             position=(1.1, 1.5, 0),
        #             scale=(0.1, 0.1, 0.1),
        #             model='cube', 
        #             material='red_pink',
        #             name='cutter bulb'
        #         ),
        #         self.node_handler.create(
        #             position=(1.35, 0.2, 0),
        #             scale=(0.15, 0.15, 0.7),
        #             model='cube', 
        #             material='grey',
        #             name='cutter barrel'
        #         ),
        #         self.node_handler.create(
        #             position=(1.35, 0.2, 0.66),
        #             scale=(0.1, 0.1, 0.1),
        #             model='cube', 
        #             material='red_pink',
        #             name='cutter emitter'
        #         ),
                
        #         # face
        #         self.node_handler.create(
        #             position=(0, -0.3, 1.02),
        #             scale=(0.4, 0.1, 0.1),
        #             model='cube', 
        #             material='black',
        #             name='mouth'
        #         ),
        #         self.node_handler.create(
        #             position=(0.6, 0, 1.02),
        #             scale=(0.2, 0.1, 0.2),
        #             rotation=(glm.pi()/2, 0, 0),
        #             model='cylinder', 
        #             material='black',
        #             name='left eye'
        #         ),
        #         self.node_handler.create(
        #             position=(0.63, 0.03, 1.03),
        #             scale=(0.1, 0.1, 0.1),
        #             rotation=(glm.pi()/2, 0, 0),
        #             model='cylinder', 
        #             material='white',
        #             name='left pupil'
        #         ),
        #         self.node_handler.create(
        #             position=(-0.6, 0, 1.02),
        #             scale=(0.2, 0.1, 0.2),
        #             rotation=(glm.pi()/2, 0, 0),
        #             model='cylinder', 
        #             material='black',
        #             name='right eye'
        #         ),
        #         self.node_handler.create(
        #             position=(-0.57, 0.03, 1.03),
        #             scale=(0.1, 0.1, 0.1),
        #             rotation=(glm.pi()/2, 0, 0),
        #             model='cylinder', 
        #             material='white',
        #             name='right pupil'
        #         ),
                
        #         # headwear
        #         self.node_handler.create(
        #             position=(0, 0.5, 0),
        #             scale=(1.1, 0.05, 1.1),
        #             model='cube', 
        #             material='baby_blue',
        #             name='lower band'
        #         ),
        #         self.node_handler.create(
        #             position=(0, 0.6, 0),
        #             scale=(1.1, 0.05, 1.1),
        #             model='cube',
        #             material='yellow',
        #             name='middle band'
        #         ),
        #         self.node_handler.create(
        #             position=(0, 0.7, 0),
        #             scale=(1.1, 0.05, 1.1),
        #             model='cube', 
        #             material='baby_blue',
        #             name='upper band'
        #         ),
        #     ]
        # )
        
        # head.on_frame = head_on_frame
        
        # john_bottom=self.skeleton_handler.add(
        #     node=bottom,
        #     joints=[
        #         BallJoint(
        #             parent_offset=(0, 1, 0),
        #             child_offset=(0, -0.5, 0),
        #             child_bone=self.skeleton_handler.create(
        #                 node=middle,
        #                 joints=[
        #                     BallJoint(
        #                         parent_offset=(0, 0.5, 0),
        #                         child_offset=(0, -0.5, 0),
        #                         child_bone=self.skeleton_handler.create(
        #                             node=top,
        #                             joints=[
        #                                 BallJoint(
        #                                     parent_offset=(0, 0.5, 0),
        #                                     child_offset=(0, -0.7, 0),
        #                                     child_bone=self.skeleton_handler.create(
        #                                         node=head
        #                                     )
        #                                 ),
        #                                 BallJoint(
        #                                     parent_offset=(-1.4, 0, 0),
        #                                     child_offset=(0, 1, 0),
        #                                     child_bone=self.skeleton_handler.create(
        #                                         node=right_arm
        #                                     )
        #                                 ),
        #                                 BallJoint(
        #                                     parent_offset=(1.4, 0, 0),
        #                                     child_offset=(0, 1, 0),
        #                                     child_bone=self.skeleton_handler.create(
        #                                         node=left_arm
        #                                     )
        #                                 )
        #                             ]
        #                         )
        #                     )
        #                 ]
        #             )
        #         ),
        #         BallJoint(
        #             parent_offset=(-0.6, 0.1, 0),
        #             child_offset=(0, 0.3, 0),
        #             spring_constant=1e4,
        #             child_bone=self.skeleton_handler.create(
        #                 node=left_knee,
        #                 joints=[
        #                     BallJoint(
        #                         parent_offset=(0, -0.3, 0),
        #                         child_offset=(0, 0.5, 0),
        #                         spring_constant = 1e4,
        #                         child_bone=self.skeleton_handler.create(
        #                             node=left_foot
        #                         )
        #                     )
        #                 ]
        #             )
        #         ),
        #         BallJoint(
        #             parent_offset=(0.6, 0.1, 0),
        #             child_offset=(0, 0.3, 0),
        #             spring_constant=1e4,
        #             child_bone=self.skeleton_handler.create(
        #                 node=right_knee,
        #                 joints=[
        #                     BallJoint(
        #                         parent_offset=(0, -0.3, 0),
        #                         child_offset=(0, 0.5, 0),
        #                         spring_constant = 1e4,
        #                         child_bone=self.skeleton_handler.create(
        #                             node=right_foot
        #                         )
        #                     )
        #                 ]
        #             )
        #         ),
        #     ]
        # )
        
        # john_bottom.on_frame = walking_animation
        
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

        if self.engine.keys[pg.K_LCTRL] and self.engine.keys[pg.K_s]:
            if platform.system() == "Darwin": 
                file = input("Enter file path: ")
                if not file.endswith('.gltf'): file += ".gltf"
            else: file = save_file_selector()
            if file: save_scene(self, abs_file_path=file)

        if self.engine.keys[pg.K_LCTRL] and self.engine.keys[pg.K_l]:
            print(platform.system())
            if platform.system() == "Darwin": 
                file = input("Enter file path: ")
                if not file.endswith('.gltf'): file += ".gltf"
            else: file = load_file_selector()
            if file:
                load_scene(self, abs_file_path=file)
                self.vao_handler.shader_handler.write_all_uniforms()
                self.project.texture_handler.write_textures()
                self.project.texture_handler.write_textures('batch')
                self.light_handler.write('batch')
                self.material_handler.write('batch')

        self.model_handler.update()
        self.vao_handler.shader_handler.update_uniforms()
        if camera: self.camera.update()
        if self.on_frame and camera: exec(self.on_frame)

    def render(self, display=True):
        """
        Redners all instances
        """

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
        
    def get_model_node_at(self, x:int, y:int, distance:float=1e5, has_collider:bool=False, has_physics_body:bool=False, material:str=None):
        """
        Gets the closest node at the pixel position
        """
        best_model = None
        best_node  = None
        pixel_position = glm.vec2(x, y)
        
        for root in self.node_handler.nodes:
            nodes = root.get_nodes(True, has_collider, has_physics_body, material)
            for node in nodes: 
                temp_model = best_model
                best_model = self.is_best_model(best_model, pixel_position, node.model, distance)
                if best_model != temp_model: best_node = node
                
        return best_node
        
    def get_model_at(self, x:int, y:int, distance:float=1e5):
        """
        Gets the closest model at pixel position
        """
        # gets the clicked model
        best_model = None
        pixel_position = glm.vec2(x, y)
        
        for model in self.model_handler.models:
            best_model = self.is_best_model(best_model, pixel_position, model, distance)

        return best_model
    
    def is_best_model(self, best_model, pixel_position, model, distance):
        if glm.dot(model.position - self.camera.position, self.camera.forward) < 0: return best_model # continue if model center behind camera TODO may not be the best

        # get model matrix 
        matrix = get_model_matrix(model.position, model.scale, model.rotation)
        model_vertices = self.model_handler.vbos[model.vbo].unique_points
        
        # print('model vertices', model_vertices)
        
        for triangle in self.model_handler.vbos[model.vbo].indicies:
            points = []
            # print(triangle, len(self.model_handler.vbos[model.vbo].unique_points))
            for point in [model_vertices[t] for t in triangle]:
                
                # remove points behind the clip plane
                vertex = glm.vec4(*point, 1.0)
                clip_space = self.camera.m_proj * self.camera.m_view * matrix * vertex
                if clip_space.z <= -clip_space.w or clip_space.w == 0: break
                
                # add point in 2d space
                ndc = clip_space / clip_space.w
                points.append(glm.vec2(
                    int((ndc.x + 1) * 0.5 * self.engine.win_size[0]),
                    int((1 - ndc.y) * 0.5 * self.engine.win_size[1])    
                ))
            else: 
                # determine if pixel point is inside triangle
                v = [p - points[2] for p in [points[0], points[1], pixel_position]]
                dot00, dot01, dot02, dot11, dot12 = glm.dot(v[0], v[0]), glm.dot(v[0], v[1]), glm.dot(v[0], v[2]), glm.dot(v[1], v[1]), glm.dot(v[1], v[2])
                determinant = (dot00 * dot11 - dot01 * dot01)
                if abs(determinant) < 1e-8: continue
                u = (dot11 * dot02 - dot01 * dot12) / determinant
                v = (dot00 * dot12 - dot01 * dot02) / determinant

                # check if point is in the triangle
                test_distance = glm.length(model.position - self.camera.position)
                if u >= 0 and v >= 0 and u + v <= 1 and test_distance < distance: 
                    best_model = model
                    distance = test_distance
                    
        return best_model