# click tracking
self.clicked        = False
self.click_anchor   = glm.vec2(0.0)
self.click_position = glm.vec2(0.0)

# camera rotation toggle
self.real_rotate = self.camera.rotate

# click identification
self.clicked_model    = None
self.clicked_node     = None
self.clicked_skeleton = None

# Rendering scripts
self.overlay = Overlay(self)