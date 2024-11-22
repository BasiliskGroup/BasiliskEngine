keys = self.node_handler.scene.engine.keys
if keys[pg.K_w] or keys[pg.K_s] or keys[pg.K_a] or keys[pg.K_d] or keys[pg.K_SPACE]:
    keys = self.node_handler.scene.engine.keys
    cam = self.node_handler.scene.camera
    self.physics_body.rotation = glm.angleAxis(glm.radians(cam.yaw - 90), cam.UP)
    self.physics_body.rotational_velocity = 0
    self.saved_rotation = self.physics_body.rotation
else: 
    self.physics_body.rotation = glm.quat(self.saved_rotation)

if self.node_handler.scene.level == 5:
    for node in self.collider.collision_normals.keys():
        if node.tags != 'cuttable': continue
        
        bottom = self.node_handler.scene.skeleton_handler.skeletons[0].node
        force = bottom.position - node.position
        force[1] = 0
        glm.normalize(force)
        
        bottom.apply_offset_force(force * 100, glm.vec3(0, 0, 0), 1)
        
        break