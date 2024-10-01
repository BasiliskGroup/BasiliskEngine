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
        
        self.skeleton_handler.add(
            node=self.node_handler.add(
                position=(0, 10, 0),
                scale=(1.5, 0.5, 0.5),
                rotation=(0, 0, 0),
                model=self.model_handler.add(vbo='cube', material='base'),
                collider=self.collider_handler.add(vbo='cube', static=False),
                physics_body=self.physics_body_handler.add(mass=5),
                name='hip'
            ),
            joints=[
                BallJoint(
                    parent_offset=(0, 1, 0),
                    child_offset=(0, -1, 0),
                    child_bone=self.skeleton_handler.create(
                        node=self.node_handler.add(
                            position=(0, 12, 0),
                            scale=(1.5, 0.5, 0.5),
                            rotation=(0, 0, 0),
                            model=self.model_handler.add(vbo='cube', material='base'),
                            collider=self.collider_handler.add(vbo='cube', static=False),
                            physics_body=self.physics_body_handler.add(mass=5),
                            name='chest'
                        ),
                        joints=[
                            BallJoint(
                                parent_offset=(0, 1, 0),
                                child_offset=(0, -1, 0),
                                child_bone=self.skeleton_handler.create(
                                    node=self.node_handler.add(
                                        position=(0, 14, 0),
                                        scale=(0.5, 0.5, 0.5),
                                        rotation=(0, 0, 0),
                                        model=self.model_handler.add(vbo='cube', material='base'),
                                        collider=self.collider_handler.add(vbo='cube', static=False),
                                        physics_body=self.physics_body_handler.add(mass=5),
                                        name='head'
                                    )
                                ),
                            ),
                            BallJoint(
                                parent_offset=(2, 0, 0),
                                child_offset=(-2, 0, 0),
                                child_bone=self.skeleton_handler.create(
                                    node=self.node_handler.add(
                                        position=(4, 12, 0),
                                        scale=(1.5, 0.5, 0.5),
                                        rotation=(0, 0, 0),
                                        model=self.model_handler.add(vbo='cube', material='base'),
                                        collider=self.collider_handler.add(vbo='cube', static=False),
                                        physics_body=self.physics_body_handler.add(mass=5),
                                        name='left arm'
                                    )
                                ),
                            ),
                            BallJoint(
                                parent_offset=(-2, 0, 0),
                                child_offset=(2, 0, 0),
                                child_bone=self.skeleton_handler.create(
                                    node=self.node_handler.add(
                                        position=(-4, 12, 0),
                                        scale=(1.5, 0.5, 0.5),
                                        rotation=(0, 0, 0),
                                        model=self.model_handler.add(vbo='cube', material='base'),
                                        collider=self.collider_handler.add(vbo='cube', static=False),
                                        physics_body=self.physics_body_handler.add(mass=5),
                                        name='right arm'
                                    )
                                ),
                            )
                        ]
                    ),
                ),
                # self.skeleton_handler.create_joint(
                #     joint_type='ball',
                #     parent_offset=(1.5, -1, 0),
                #     child_offset=(0, 2, 0),
                #     child_bone=self.skeleton_handler.create(
                #         node=self.node_handler.add(
                #             position=(2, 7, 0),
                #             scale=(0.5, 1.5, 0.5),
                #             rotation=(0, 0, 0),
                #             model=self.model_handler.add(vbo='cube', material='base'),
                #             collider=self.collider_handler.add(vbo='cube', static=False),
                #             physics_body=self.physics_body_handler.add(mass=5),
                #             name='left leg'
                #         )
                #     ),
                # ),
                # self.skeleton_handler.create_joint(
                #     joint_type='ball',
                #     parent_offset=(-2, -1, 0),
                #     child_offset=(0, 2, 0),
                #     child_bone=self.skeleton_handler.create(
                #         node=self.node_handler.add(
                #             position=(-2, 7, 0),
                #             scale=(0.5, 1.5, 0.5),
                #             rotation=(0, 0, 0),
                #             model=self.model_handler.add(vbo='cube', material='base'),
                #             collider=self.collider_handler.add(vbo='cube', static=False),
                #             physics_body=self.physics_body_handler.add(mass=5),
                #             name='right leg'
                #         )
                #     ),
                # )
            ]
        )
        
        self.node_handler.add(
            position=(0, -5, 0),
            scale=(40, 1, 40),
            rotation=(0, 0, 0),
            nodes=[],
            model=self.model_handler.add('cube', 'base'),
            collider=self.collider_handler.add(vbo='cube', static=True),
            physics_body=None,
            name='box'
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