import pygame as pg
from scripts.camera import Camera
from scripts.model_handler import ModelHandler
from scripts.collisions.collider_handler import ColliderHandler
from scripts.physics.physics_body_handler import PhysicsBodyHandler
from scripts.nodes.node_handler import NodeHandler
from scripts.skeletons.skeleton_handler import SkeletonHandler
from scripts.render.material_handler import MaterialHandler
from scripts.render.light_handler import LightHandler
from scripts.skeletons.joints import *
from random import uniform, randint
from scripts.render.sky import Sky


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
        self.camera = Camera(self.engine)

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
        
        spacing = 6

        models = ['cube', 'cow']

        '''for x in range(0, 5):
            for y in range(0, 5):
                for z in range(0, 5):
                    self.model_handler.add(models[randrange(0, 2)], "cow", (x * spacing, y * spacing, z * spacing), (0, 0, 0), (1, 1, 1))'''

        self.selected_model = self.model_handler.add("cow", "base", (4 * spacing, 4 * spacing, 4 * spacing), (0, 0, 0), (3, 3, 3))
        
        self.node_handler.add(
            position=(0, -4, 0),
            scale=(40, 1, 40),
            rotation=(0, 0, 0),
            nodes=[],
            model=self.model_handler.add('cube', 'base'),
            collider=self.collider_handler.add(vbo='cube', static=True),
            physics_body=None,
            name='box'
        )
        
        cock_pos = glm.vec3(0, -2, 0)
        
        bottom=self.node_handler.add(
            position=cock_pos + glm.vec3(0, 2, 0),
            scale=(1, 1, 1),
            rotation=(0, 0, 0),
            collider=self.collider_handler.add(vbo='cube', static=False),
            physics_body=self.physics_body_handler.add(mass=200),
            nodes=[
                self.node_handler.create(
                    position=glm.vec3(0, 0.5, 0),
                    scale=(1, 0.5, 1),
                    rotation=(0, 0, 0),
                    model=self.model_handler.add(vbo='cube', material='white'),
                )
            ],
            name='bottom'
        )
        
        middle=self.node_handler.add(
            position=cock_pos + glm.vec3(0, 3.5, 0),
            scale=(1, 0.5, 1),
            rotation=(0, 0, 0),
            model=self.model_handler.add(vbo='cube', material='white'),
            collider=self.collider_handler.add(vbo='cube', static=False),
            physics_body=self.physics_body_handler.add(mass=20),
            name='middle'
        )
        
        top=self.node_handler.add(
            position=cock_pos + glm.vec3(0, 4.5, 0),
            scale=(1, 0.5, 1),
            rotation=(0, 0, 0),
            model=self.model_handler.add(vbo='cube', material='white'),
            collider=self.collider_handler.add(vbo='cube', static=False),
            physics_body=self.physics_body_handler.add(mass=20),
            name='top'
        )
        
        left_arm=self.node_handler.add(
            position=cock_pos + glm.vec3(1.3, 3.5, 0),
            scale=(0.3, 1.25, 0.3),
            rotation=(0, 0, 0),
            model=self.model_handler.add(vbo='cube', material='white'),
            collider=self.collider_handler.add(vbo='cube', static=False),
            physics_body=self.physics_body_handler.add(mass=20),
            name='left arm',
            nodes=[
                # arm puff
                self.node_handler.add(
                    position=(0, -0.64, 0),
                    scale=(1.1, 0.03, 1.1),
                    model=self.model_handler.add(vbo='cube', material='baby_blue'),
                    name='arm puff'
                ),
                self.node_handler.add(
                    position=(0, -0.7, 0),
                    scale=(1.1, 0.03, 1.1),
                    model=self.model_handler.add(vbo='cube', material='yellow'),
                    name='arm puff'
                ),
                self.node_handler.add(
                    position=(0, -0.76, 0),
                    scale=(1.1, 0.03, 1.1),
                    model=self.model_handler.add(vbo='cube', material='baby_blue'),
                    name='arm puff'
                ),
            ]
        )
        
        right_arm=self.node_handler.add(
            position=cock_pos + glm.vec3(-1.3, 3.5, 0),
            scale=(0.3, 1.25, 0.3),
            rotation=(0, 0, 0),
            model=self.model_handler.add(vbo='cube', material='white'),
            collider=self.collider_handler.add(vbo='cube', static=False),
            physics_body=self.physics_body_handler.add(mass=20),
            name='right arm',
            nodes=[
                # arm puff
                self.node_handler.add(
                    position=(0, -0.64, 0),
                    scale=(1.1, 0.03, 1.1),
                    model=self.model_handler.add(vbo='cube', material='baby_blue'),
                    name='arm puff'
                ),
                self.node_handler.add(
                    position=(0, -0.7, 0),
                    scale=(1.1, 0.03, 1.1),
                    model=self.model_handler.add(vbo='cube', material='yellow'),
                    name='arm puff'
                ),
                self.node_handler.add(
                    position=(0, -0.76, 0),
                    scale=(1.1, 0.03, 1.1),
                    model=self.model_handler.add(vbo='cube', material='baby_blue'),
                    name='arm puff'
                ),
            ]
        )
        
        head=self.node_handler.add(
            position=cock_pos + glm.vec3(0, 5.7, 0),
            scale=(0.7, 0.7, 0.7),
            rotation=(0, 0, 0),
            model=self.model_handler.add(vbo='cube', material='white'),
            collider=self.collider_handler.add(vbo='cube', static=False),
            physics_body=self.physics_body_handler.add(mass=20),
            name='head',
            nodes=[
                # cutter
                self.node_handler.create(
                    position=(1, 0, 0),
                    scale=(0.2, 0.5, 0.5),
                    model=self.model_handler.add(vbo='cube', material='grey'),
                    name='cutter base'
                ),
                self.node_handler.create(
                    position=(1.1, 1, 0),
                    scale=(0.05, 0.5, 0.05),
                    model=self.model_handler.add(vbo='cube', material='dark_grey'),
                    name='cutter antenna'
                ),
                self.node_handler.create(
                    position=(1.1, 1.5, 0),
                    scale=(0.1, 0.1, 0.1),
                    model=self.model_handler.add(vbo='cube', material='red_pink'),
                    name='cutter bulb'
                ),
                self.node_handler.create(
                    position=(1.35, 0.2, 0),
                    scale=(0.15, 0.15, 0.7),
                    model=self.model_handler.add(vbo='cube', material='grey'),
                    name='cutter barrel'
                ),
                self.node_handler.create(
                    position=(1.35, 0.2, 0.66),
                    scale=(0.1, 0.1, 0.1),
                    model=self.model_handler.add(vbo='cube', material='red_pink'),
                    name='cutter emitter'
                ),
                
                # face
                self.node_handler.create(
                    position=(0, -0.3, 1.02),
                    scale=(0.4, 0.1, 0.1),
                    model=self.model_handler.add(vbo='cube', material='black'),
                    name='mouth'
                ),
                self.node_handler.create(
                    position=(0.6, 0, 1.02),
                    scale=(0.2, 0.1, 0.2),
                    rotation=(glm.pi()/2, 0, 0),
                    model=self.model_handler.add(vbo='cylinder', material='black'),
                    name='left eye'
                ),
                self.node_handler.create(
                    position=(0.63, 0.03, 1.03),
                    scale=(0.1, 0.1, 0.1),
                    rotation=(glm.pi()/2, 0, 0),
                    model=self.model_handler.add(vbo='cylinder', material='white'),
                    name='left pupil'
                ),
                self.node_handler.create(
                    position=(-0.6, 0, 1.02),
                    scale=(0.2, 0.1, 0.2),
                    rotation=(glm.pi()/2, 0, 0),
                    model=self.model_handler.add(vbo='cylinder', material='black'),
                    name='right eye'
                ),
                self.node_handler.create(
                    position=(-0.57, 0.03, 1.03),
                    scale=(0.1, 0.1, 0.1),
                    rotation=(glm.pi()/2, 0, 0),
                    model=self.model_handler.add(vbo='cylinder', material='white'),
                    name='right pupil'
                ),
                
                # headwear
                self.node_handler.create(
                    position=(0, 0.5, 0),
                    scale=(1.1, 0.05, 1.1),
                    model=self.model_handler.add(vbo='cube', material='baby_blue'),
                    name='lower band'
                ),
                self.node_handler.create(
                    position=(0, 0.6, 0),
                    scale=(1.1, 0.05, 1.1),
                    model=self.model_handler.add(vbo='cube', material='yellow'),
                    name='middle band'
                ),
                self.node_handler.create(
                    position=(0, 0.7, 0),
                    scale=(1.1, 0.05, 1.1),
                    model=self.model_handler.add(vbo='cube', material='baby_blue'),
                    name='upper band'
                ),
            ]
        )
        
        left_foot=self.node_handler.add(
            position=cock_pos + glm.vec3(0.5, 1.5, 0),
            scale=(0.3, 0.5, 0.3),
            rotation=(0, 0, 0),
            model=self.model_handler.add(vbo='cube', material='white'),
            physics_body=self.physics_body_handler.add(mass=20),
            name='left foot',
            nodes=[
                # leg puff
                self.node_handler.add(
                    position=(0, -0.6, 0),
                    scale=(1.1, 0.05, 1.1),
                    model=self.model_handler.add(vbo='cube', material='baby_blue'),
                    name='leg puff'
                ),
                self.node_handler.add(
                    position=(0, -0.5, 0),
                    scale=(1.1, 0.05, 1.1),
                    model=self.model_handler.add(vbo='cube', material='yellow'),
                    name='leg puff'
                ),
                self.node_handler.add(
                    position=(0, -0.4, 0),
                    scale=(1.1, 0.05, 1.1),
                    model=self.model_handler.add(vbo='cube', material='baby_blue'),
                    name='leg puff'
                )
            ]
        )
        
        right_foot=self.node_handler.add(
            position=cock_pos + glm.vec3(-0.5, 1.5, 0),
            scale=(0.3, 0.5, 0.3),
            rotation=(0, 0, 0),
            model=self.model_handler.add(vbo='cube', material='white'),
            physics_body=self.physics_body_handler.add(mass=20),
            name='left foot',
            nodes=[
                # leg puff
                self.node_handler.add(
                    position=(0, -0.6, 0),
                    scale=(1.1, 0.05, 1.1),
                    model=self.model_handler.add(vbo='cube', material='baby_blue'),
                    name='leg puff'
                ),
                self.node_handler.add(
                    position=(0, -0.5, 0),
                    scale=(1.1, 0.05, 1.1),
                    model=self.model_handler.add(vbo='cube', material='yellow'),
                    name='leg puff'
                ),
                self.node_handler.add(
                    position=(0, -0.4, 0),
                    scale=(1.1, 0.05, 1.1),
                    model=self.model_handler.add(vbo='cube', material='baby_blue'),
                    name='leg puff'
                )
            ]
        )
        
        self.skeleton_handler.add(
            node=bottom,
            joints=[
                BallJoint(
                    parent_offset=(0, 1, 0),
                    child_offset=(0, -0.5, 0),
                    child_bone=self.skeleton_handler.add(
                        node=middle,
                        joints=[
                            BallJoint(
                                parent_offset=(0, 0.5, 0),
                                child_offset=(0, -0.5, 0),
                                child_bone=self.skeleton_handler.add(
                                    node=top,
                                    joints=[
                                        BallJoint(
                                            parent_offset=(0, 0.5, 0),
                                            child_offset=(0, -0.7, 0),
                                            child_bone=self.skeleton_handler.add(
                                                node=head
                                            )
                                        ),
                                        BallJoint(
                                            parent_offset=(-1, 0, 0),
                                            child_offset=(0.3, 1, 0),
                                            child_bone=self.skeleton_handler.add(
                                                node=right_arm
                                            )
                                        ),
                                        BallJoint(
                                            parent_offset=(1, 0, 0),
                                            child_offset=(-0.3, 1, 0),
                                            child_bone=self.skeleton_handler.add(
                                                node=left_arm
                                            )
                                        )
                                    ]
                                )
                            )
                        ]
                    )
                ),
                BallJoint(
                    parent_offset=(0.5, 0, 0),
                    child_offset=(0, 0.5, 0),
                    child_bone=self.skeleton_handler.add(
                        node=left_foot
                    )
                ),
                BallJoint(
                    parent_offset=(-0.5, 0, 0),
                    child_offset=(0, 0.5, 0),
                    child_bone=self.skeleton_handler.add(
                        node=right_foot
                    )
                )
            ]
        )
        
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

    def render(self, display=True):
        """
        Redners all instances
        """

        self.vao_handler.framebuffer.clear(color=(0.08, 0.16, 0.18, 1.0))
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