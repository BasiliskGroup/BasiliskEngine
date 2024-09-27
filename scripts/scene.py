import pygame as pg
from scripts.camera import Camera
from scripts.model_handler import ModelHandler
from scripts.collisions.collider_handler import ColliderHandler
from scripts.physics.physics_body_handler import PhysicsBodyHandler
from scripts.nodes.node_handler import NodeHandler
from scripts.skeletons.skeleton_handler import SkeletonHandler
from scripts.skeletons.joints import *
from random import uniform, randint

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
        self.model_handler = ModelHandler(self)
        self.collider_handler = ColliderHandler(self)
        self.physics_body_handler = PhysicsBodyHandler(self)
        self.node_handler = NodeHandler(self)
        self.skeleton_handler = SkeletonHandler(self)

        spacing = 6

        models = ['cube', 'cow']

        '''for x in range(0, 5):
            for y in range(0, 5):
                for z in range(0, 5):
                    self.model_handler.add(models[randrange(0, 2)], "cow", (x * spacing, y * spacing, z * spacing), (0, 0, 0), (1, 1, 1))'''

        self.selected_model = self.model_handler.add("cow", "box", (4 * spacing, 4 * spacing, 4 * spacing), (0, 0, 0), (3, 3, 3))
        
        self.skeleton_handler.add(
            node=self.node_handler.add(
                position=(0, 10, 0),
                scale=(1.5, 0.5, 0.5),
                rotation=(0, 0, 0),
                model=self.model_handler.add(vbo='cube', texture='box'),
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
                            model=self.model_handler.add(vbo='cube', texture='box'),
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
                                        model=self.model_handler.add(vbo='cube', texture='box'),
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
                                        model=self.model_handler.add(vbo='cube', texture='box'),
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
                                        model=self.model_handler.add(vbo='cube', texture='box'),
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
                #             model=self.model_handler.add(vbo='cube', texture='box'),
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
                #             model=self.model_handler.add(vbo='cube', texture='box'),
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
            model=self.model_handler.add('cube', 'box'),
            collider=self.collider_handler.add(vbo='cube', static=True),
            physics_body=None,
            name='box'
        )
        
        self.collider_handler.construct_bvh()
                
    def use(self):
        """
        Updates project handlers to use this scene
        """

        self.vao_handler.shader_handler.set_camera(self.camera)
        self.camera.use()
        self.vao_handler.shader_handler.write_all_uniforms()
        self.project.texture_handler.write_textures()
        self.project.texture_handler.write_textures('batch')

    def update(self):
        """
        Updates uniforms, and camera
        """
        
        if self.engine.keys[pg.K_e]:
            place_range = 100
            for i in range(100):
                self.model_handler.add(position=(self.camera.position.x + randint(-place_range, place_range), self.camera.position.y + randint(-place_range, place_range), self.camera.position.z + randint(-place_range, place_range)))
        
        self.selected_model.scale.x += (self.engine.keys[pg.K_UP] - self.engine.keys[pg.K_DOWN]) * self.engine.dt * 10
        self.selected_model.scale.z += (self.engine.keys[pg.K_RIGHT] - self.engine.keys[pg.K_LEFT]) * self.engine.dt * 10

        self.model_handler.update()
        self.vao_handler.shader_handler.update_uniforms()
        self.camera.update()

    def render(self):
        """
        Redners all instances
        """

        self.ctx.screen.use()
        self.model_handler.render()

    def release(self):
        """
        Releases scene's VAOs
        """

        self.vao_handler.release()