import glm
from scripts.skeletons.animation import *
from scripts.skeletons.joints import *

# Add all the materials needed for john
if "baby_blue" not in self.material_handler.materials: self.material_handler.add("baby_blue", color=(.69, .97, 1), specular=.5, specular_exponent=64)
if "white" not in self.material_handler.materials:     self.material_handler.add("white", color=(1.0, 1.0, 1.0), specular=.5, specular_exponent=64)
if "grey" not in self.material_handler.materials:   self.material_handler.add("grey", color=(.72, .72, .72), specular=.5, specular_exponent=64)
if "red_pink" not in self.material_handler.materials: self.material_handler.add("red_pink", color=(.99, .01, .15), specular=.5, specular_exponent=64)
if "black" not in self.material_handler.materials: self.material_handler.add("black", color=(0.0, 0.0, 0.0), specular=.5, specular_exponent=64)
if "dark_grey" not in self.material_handler.materials: self.material_handler.add("dark_grey", color=(.29, .29, .29), specular=.5, specular_exponent=64)
if "yellow" not in self.material_handler.materials: self.material_handler.add("yellow", color=(1, .96, .22), specular=.5, specular_exponent=64)


cock_pos = glm.vec3(0, -2, 0)
left_foot=self.node_handler.add(
    position=cock_pos + glm.vec3(0.5, 0.25, 0),
    scale=(0.3, 0.25, 0.3),
    rotation=(0, 0, 0),
    model='cube', 
    material='white',
    physics_body=self.physics_body_handler.add(mass=5),
    name='left foot',
    
    nodes=[
        # leg puff
        self.node_handler.create(
            position=(0, -0.6, 0),
            scale=(1.1, 0.05, 1.1),
            model='cube', 
            material='baby_blue',
            name='leg puff'
        ),
        self.node_handler.create(
            position=(0, -0.5, 0),
            scale=(1.1, 0.05, 1.1),
            model='cube', 
            material='yellow',
            name='leg puff'
        ),
        self.node_handler.create(
            position=(0, -0.4, 0),
            scale=(1.1, 0.05, 1.1),
            model='cube', 
            material='baby_blue',
            name='leg puff'
        )
    ]
)

left_foot.on_frame=self.face_camera

left_knee=self.node_handler.add(
    position=cock_pos + glm.vec3(0.5, 0.75, 0),
    scale=(0.3, 0.7, 0.3),
    rotation=(0, 0, 0),
    model='cube', 
    material='white',
    physics_body=self.physics_body_handler.add(mass=5),
    name='left foot',
)

right_foot=self.node_handler.add(
    position=cock_pos + glm.vec3(-0.5, 0.25, 0),
    scale=(0.3, 0.25, 0.3),
    rotation=(0, 0, 0),
    model='cube',
    material='white',
    physics_body=self.physics_body_handler.add(mass=5),
    name='right foot',
    
    nodes=[
        # leg puff
        self.node_handler.create(
            position=(0, -0.6, 0),
            scale=(1.1, 0.05, 1.1),
            model='cube', 
            material='baby_blue',
            name='leg puff'
        ),
        self.node_handler.create(
            position=(0, -0.5, 0),
            scale=(1.1, 0.05, 1.1),
            model='cube', 
            material='yellow',
            name='leg puff'
        ),
        self.node_handler.create(
            position=(0, -0.4, 0),
            scale=(1.1, 0.05, 1.1),
            model='cube',
            material='baby_blue',
            name='leg puff'
        )
    ]
)

right_foot.on_frame=self.face_camera

right_knee=self.node_handler.add(
    position=cock_pos + glm.vec3(-0.5, 0.75, 0),
    scale=(0.3, 0.7, 0.3),
    rotation=(0, 0, 0),
    model='cube', 
    material='white',
    physics_body=self.physics_body_handler.add(mass=5),
    name='left foot',
)

bottom=self.node_handler.add(
    position=cock_pos + glm.vec3(0, 0.5, 0),
    scale=(1, 1.5, 1),
    rotation=(0, 0, 0),
    collider=self.collider_handler.add(vbo='cube', static=False, group='john'),
    physics_body=self.physics_body_handler.add(mass=100),
    nodes=[
        self.node_handler.create(
            position=(-2, 6, -2),
            camera=self.camera
        ),
        self.node_handler.create(
            position=glm.vec3(0, 0.5, 0),
            scale=(1, 0.25, 1),
            rotation=(0, 0, 0),
            model='cube', 
            material='white',
        )
    ],
    name='bottom'
)

bottom.on_frame = self.bottom_on_frame
setattr(bottom, 'jump_time', 0)
setattr(bottom, 'jump_max', 1)

middle=self.node_handler.add(
    position=cock_pos + glm.vec3(0, 2.5, 0),
    scale=(1, 0.5, 1),
    rotation=(0, 0, 0),
    model='cube', 
    material='white',
    collider=self.collider_handler.add(vbo='cube', static=False, group='john'),
    physics_body=self.physics_body_handler.add(mass=10),
    name='middle',
    
    nodes=[
        self.node_handler.create(
            position=glm.vec3(0, 0, 1),
            scale=(0.8, 0.8, 0.01),
            rotation=(0, 0, 0),
            model='cube', 
            material='grey',
        ),
        self.node_handler.create(
            position=glm.vec3(0, 0.1, 1),
            scale=(0.6, 0.6, 0.015),
            rotation=(0, 0, 0),
            model='cube', 
            material='white',
        ),
        self.node_handler.create(
            position=glm.vec3(0, 0.1, 1),
            scale=(0.6, 0.3, 0.02),
            rotation=(0, 0, 0),
            model='cube', 
            material='baby_blue',
        ),
        self.node_handler.create(
            position=glm.vec3(0, 0.1, 1),
            scale=(0.35, 0.15, 0.025),
            rotation=(0, 0, 0),
            model='cube', 
            material='black',
        ),
        self.node_handler.create(
            position=(0.2, 0.1, 1.02),
            scale=(0.08, 0.03, 0.08),
            rotation=(glm.pi()/2, 0, 0),
            model='cylinder', 
            material='white',
        ),
        self.node_handler.create(
            position=(-0.2, 0.1, 1.02),
            scale=(0.08, 0.03, 0.08),
            rotation=(glm.pi()/2, 0, 0),
            model='cylinder', 
            material='white',
        ),
    ]
)

middle.on_frame = self.face_camera

top=self.node_handler.add(
    position=cock_pos + glm.vec3(0, 3.5, 0),
    scale=(1, 0.5, 1),
    rotation=(0, 0, 0),
    model='cube', 
    material='baby_blue',
    collider=self.collider_handler.add(vbo='cube', static=False, group='john'),
    physics_body=self.physics_body_handler.add(mass=10),
    name='top',
    
    nodes=[
        # id
        self.node_handler.create(
            position=glm.vec3(0.5, 0.2, 1),
            scale=(0.3, 0.1, 0.05),
            rotation=(0, 0, -0.1),
            model='cube', 
            material='red_pink',
        ),
        self.node_handler.create(
            position=glm.vec3(0.51, 0, 1),
            scale=(0.3, 0.1, 0.05),
            rotation=(0, 0, -0.1),
            model='cube', 
            material='white',
        ),
        self.node_handler.create(
            position=glm.vec3(0.52, -0.2, 1),
            scale=(0.3, 0.1, 0.05),
            rotation=(0, 0, -0.1),
            model='cube', 
            material='red_pink',
        ),
        self.node_handler.create(
            position=glm.vec3(0.5, 0.15, 1),
            scale=(0.1, 0.03, 0.055),
            rotation=(0, 0, 0.1),
            model='cube', 
            material='black',
        ),
        self.node_handler.create(
            position=glm.vec3(0.5, -0.05, 1),
            scale=(0.03, 0.2, 0.055),
            rotation=(0, 0, 0.1),
            model='cube', 
            material='black',
        ),
        self.node_handler.create(
            position=glm.vec3(0.425, -0.15, 1),
            scale=(0.05, 0.03, 0.055),
            rotation=(0, 0, 0.6),
            model='cube', 
            material='black',
        ),
        self.node_handler.create(
            position=glm.vec3(0.5, 0.25, 1.05),
            scale=(0.05, 0.1, 0.05),
            rotation=(glm.pi()/2, 0, 0),
            model='cylinder', 
            material='grey',
        ),
        # buttons
        
    ]
)

top.on_frame = self.face_camera

left_arm=self.node_handler.add(
    position=cock_pos + glm.vec3(1.3, 2.5, 0),
    scale=(0.3, 1.25, 0.3),
    rotation=(0, 0, 0),
    model='cube',
    material='white',
    collider=self.collider_handler.add(vbo='cube', static=False),
    physics_body=self.physics_body_handler.add(mass=10),
    name='left arm',
    
    nodes=[
        # arm puff
        self.node_handler.create(
            position=(0, -0.64, 0),
            scale=(1.1, 0.03, 1.1),
            model='cube', 
            material='baby_blue',
            name='arm puff'
        ),
        self.node_handler.create(
            position=(0, -0.7, 0),
            scale=(1.1, 0.03, 1.1),
            model='cube', 
            material='yellow',
            name='arm puff'
        ),
        self.node_handler.create(
            position=(0, -0.76, 0),
            scale=(1.1, 0.03, 1.1),
            model='cube', 
            material='baby_blue',
            name='arm puff'
        ),
    ]
)

right_arm=self.node_handler.add(
    position=cock_pos + glm.vec3(-1.3, 2.5, 0),
    scale=(0.3, 1.25, 0.3),
    rotation=(0, 0, 0),
    model='cube',
    material='white',
    collider=self.collider_handler.add(vbo='cube', static=False),
    physics_body=self.physics_body_handler.add(mass=10),
    name='right arm',
    
    nodes=[
        # arm puff
        self.node_handler.create(
            position=(0, -0.64, 0),
            scale=(1.1, 0.03, 1.1),
            model='cube', 
            material='baby_blue',
            name='arm puff'
        ),
        self.node_handler.create(
            position=(0, -0.7, 0),
            scale=(1.1, 0.03, 1.1),
            model='cube', 
            material='yellow',
            name='arm puff'
        ),
        self.node_handler.create(
            position=(0, -0.76, 0),
            scale=(1.1, 0.03, 1.1),
            model='cube', 
            material='baby_blue',
            name='arm puff'
        ),
    ]
)

head=self.node_handler.add(
    position=cock_pos + glm.vec3(0, 4.7, 0),
    scale=(0.7, 0.7, 0.7),
    rotation=(0, 0, 0),
    model='cube', 
    material='white',
    collider=self.collider_handler.add(vbo='cube', static=False, group='john'),
    physics_body=self.physics_body_handler.add(mass=10),
    name='head',
    
    nodes=[
        # cutter
        self.node_handler.create(
            position=(1, 0, 0),
            scale=(0.2, 0.5, 0.5),
            model='cube', 
            material='grey',
            name='cutter base'
        ),
        self.node_handler.create(
            position=(1.1, 1, 0),
            scale=(0.05, 0.5, 0.05),
            model='cube', 
            material='dark_grey',
            name='cutter antenna'
        ),
        self.node_handler.create(
            position=(1.1, 1.5, 0),
            scale=(0.1, 0.1, 0.1),
            model='cube', 
            material='red_pink',
            name='cutter bulb'
        ),
        self.node_handler.create(
            position=(1.35, 0.2, 0),
            scale=(0.15, 0.15, 0.7),
            model='cube', 
            material='grey',
            name='cutter barrel'
        ),
        self.node_handler.create(
            position=(1.35, 0.2, 0.66),
            scale=(0.1, 0.1, 0.1),
            model='cube', 
            material='red_pink',
            name='cutter emitter'
        ),
        
        # face
        self.node_handler.create(
            position=(0, -0.3, 1.02),
            scale=(0.4, 0.1, 0.1),
            model='cube', 
            material='black',
            name='mouth'
        ),
        self.node_handler.create(
            position=(0.6, 0, 1.02),
            scale=(0.2, 0.1, 0.2),
            rotation=(glm.pi()/2, 0, 0),
            model='cylinder', 
            material='black',
            name='left eye'
        ),
        self.node_handler.create(
            position=(0.63, 0.03, 1.03),
            scale=(0.1, 0.1, 0.1),
            rotation=(glm.pi()/2, 0, 0),
            model='cylinder', 
            material='white',
            name='left pupil'
        ),
        self.node_handler.create(
            position=(-0.6, 0, 1.02),
            scale=(0.2, 0.1, 0.2),
            rotation=(glm.pi()/2, 0, 0),
            model='cylinder', 
            material='black',
            name='right eye'
        ),
        self.node_handler.create(
            position=(-0.57, 0.03, 1.03),
            scale=(0.1, 0.1, 0.1),
            rotation=(glm.pi()/2, 0, 0),
            model='cylinder', 
            material='white',
            name='right pupil'
        ),
        
        # headwear
        self.node_handler.create(
            position=(0, 0.5, 0),
            scale=(1.1, 0.05, 1.1),
            model='cube', 
            material='baby_blue',
            name='lower band'
        ),
        self.node_handler.create(
            position=(0, 0.6, 0),
            scale=(1.1, 0.05, 1.1),
            model='cube',
            material='yellow',
            name='middle band'
        ),
        self.node_handler.create(
            position=(0, 0.7, 0),
            scale=(1.1, 0.05, 1.1),
            model='cube', 
            material='baby_blue',
            name='upper band'
        )
    ]
)

head.on_frame = self.head_on_frame

john_bottom=self.skeleton_handler.add(
    node=bottom,
    joints=[
        BallJoint(
            parent_offset=(0, 1, 0),
            child_offset=(0, -0.5, 0),
            child_bone=self.skeleton_handler.create(
                node=middle,
                joints=[
                    BallJoint(
                        parent_offset=(0, 0.5, 0),
                        child_offset=(0, -0.5, 0),
                        child_bone=self.skeleton_handler.create(
                            node=top,
                            joints=[
                                BallJoint(
                                    parent_offset=(0, 0.5, 0),
                                    child_offset=(0, -0.7, 0),
                                    child_bone=self.skeleton_handler.create(
                                        node=head
                                    )
                                ),
                                BallJoint(
                                    parent_offset=(-1.6, 0, 0),
                                    child_offset=(0, 1, 0),
                                    child_bone=self.skeleton_handler.create(
                                        node=right_arm
                                    )
                                ),
                                BallJoint(
                                    parent_offset=(1.6, 0, 0),
                                    child_offset=(-0, 1, 0),
                                    child_bone=self.skeleton_handler.create(
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
            parent_offset=(-0.6, 0.1, 0),
            child_offset=(0, 0.3, 0),
            spring_constant=1e4,
            child_bone=self.skeleton_handler.create(
                node=left_knee,
                joints=[
                    BallJoint(
                        parent_offset=(0, -0.3, 0),
                        child_offset=(0, 0.5, 0),
                        spring_constant = 1e4,
                        child_bone=self.skeleton_handler.create(
                            node=left_foot
                        )
                    )
                ]
            )
        ),
        BallJoint(
            parent_offset=(0.6, 0.1, 0),
            child_offset=(0, 0.3, 0),
            spring_constant=1e4,
            child_bone=self.skeleton_handler.create(
                node=right_knee,
                joints=[
                    BallJoint(
                        parent_offset=(0, -0.3, 0),
                        child_offset=(0, 0.5, 0),
                        spring_constant = 1e4,
                        child_bone=self.skeleton_handler.create(
                            node=right_foot
                        )
                    )
                ]
            )
        ),
    ]
)

john_bottom.on_frame = self.walking_animation

setattr(left_foot, 'saved_rotation', glm.quat())
setattr(right_foot, 'saved_rotation', glm.quat())
setattr(bottom, 'saved_rotation', glm.quat())
setattr(middle, 'saved_rotation', glm.quat())
setattr(top, 'saved_rotation', glm.quat())
setattr(head, 'saved_rotation', glm.quat())
