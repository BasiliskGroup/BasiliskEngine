import types

# Load node scripts
with open(f'user_scripts/add_john.py')          as file: add_john               = compile(file.read(), 'add_john', 'exec')
with open(f'user_scripts/bottom_on_frame.py')   as file: self.bottom_on_frame   = compile(file.read(), 'bottom_on_frame', 'exec')
with open(f'user_scripts/face_camera.py')       as file: self.face_camera       = compile(file.read(), 'face_camera', 'exec')
with open(f'user_scripts/walking_animation.py') as file: self.walking_animation = compile(file.read(), 'walking_animation', 'exec')
with open(f'user_scripts/head_on_frame.py')     as file: self.head_on_frame     = compile(file.read(), 'head_on_frame', 'exec')

# levels
self.level = 5

# john
self.add_john_compile = add_john
def add_john_def(self): exec(self.add_john_compile)
self.add_john = types.MethodType(add_john_def, self)
self.add_john()

# click tracking
self.clicked        = False
self.click_anchor   = glm.vec2(0.0)
self.click_position = glm.vec2(0.0)

# camera rotation toggle
self.real_rotate = self.camera.rotate

self.grab_distance = 0
self.grabbed = None

# click identification
self.clicked_model    = None
self.clicked_node     = None
self.clicked_skeleton = None

# Rendering scripts
self.overlay = Overlay(self)