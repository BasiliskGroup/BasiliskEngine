# pull information
nodes = [self.joints[1].child_bone.joints[0].child_bone.node, self.joints[2].child_bone.joints[0].child_bone.node]
nodes[0].position[1], nodes[1].position[1] = self.node.position[1] - 1.2, self.node.position[1] - 1.2

animated = False
for index in range(0, 2):
    if len(self.joints[index + 1].child_bone.joints[0].animations) > 0: animated = True
    else: nodes[index].physics_body.velocity = glm.vec3(0.0)
    
# operate legs
for index in range(0, 2):
    
    body_pos = self.node.position + self.joints[index + 1].parent_offset + (0, 0.5, 0)

    leg = self.joints[index + 1].child_bone.node
    leg.physics_body.rotational_velocity = 0
    
    # center legs
    foot_pos = nodes[index].position
    leg.position = (foot_pos + body_pos) / 2
    
    # orient legs
    target = glm.normalize(foot_pos - body_pos)
    origin = glm.normalize(leg.physics_body.rotation * (0, -1, 0))
    
    axis  = glm.cross(origin, target)
    angle = glm.acos(glm.clamp(glm.dot(origin, target), -1, 1))
    rot   = glm.angleAxis(angle, axis)
    leg.physics_body.rotation = glm.inverse(rot * leg.physics_body.rotation)
    
# break if feet already in animation
if not animated: # start next animation
    
    # determine walking direction
    direction = glm.vec3(0.0)
    keys = self.skeleton_handler.scene.engine.keys
    cam  = self.skeleton_handler.scene.camera
    if keys[pg.K_w]: direction += glm.normalize(glm.vec3(cam.forward.x, 0, cam.forward.z))
    if keys[pg.K_s]: direction -= glm.normalize(glm.vec3(cam.forward.x, 0, cam.forward.z))
    if keys[pg.K_a]: direction -= cam.right
    if keys[pg.K_d]: direction += cam.right
    direction = glm.normalize(direction) * 2

    # move feet
    radius = 1.5 if glm.length(direction) > 0 else 0

    points = [glm.vec3(nodes[0].position), glm.vec3(nodes[1].position)]
    centers = [self.node.position + self.joints[1].parent_offset + direction, self.node.position + self.joints[2].parent_offset + direction]

    for point in points: point[1] = 0
    for center in centers: center[1] = 0
    
    index = 1 if glm.length(centers[0] - points[0]) > glm.length(centers[1] - points[1]) else 2
    if glm.length(centers[index - 1] - points[index - 1]) > radius: 
        
        dest = centers[index - 1]
        dest[1] = self.node.position[1] - 1.2
        
        self.joints[index].child_bone.joints[0].animations.append(
            Animation(
                key_frames=[
                    KeyFrame(
                        position=dest,
                        time=0.1
                    )
                ]
            )
        )