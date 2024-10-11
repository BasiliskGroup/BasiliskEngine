keys = self.node_handler.scene.engine.keys
if keys[pg.K_w] or keys[pg.K_s] or keys[pg.K_a] or keys[pg.K_d] or keys[pg.K_SPACE] or keys[pg.K_LSHIFT] or True: 
    cam = self.node_handler.scene.camera
    self.physics_body.rotation = glm.angleAxis(glm.radians(cam.yaw - 90), cam.UP)
    self.physics_body.rotational_velocity = 0