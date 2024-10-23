keys = self.node_handler.scene.engine.keys

cam = self.node_handler.scene.camera
self.physics_body.rotation = glm.angleAxis(glm.radians(cam.yaw - 90), cam.UP)
self.physics_body.rotational_velocity = 0