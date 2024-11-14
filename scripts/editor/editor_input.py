import pygame as pg
from scripts.file_manager.save_scene import save_scene
from scripts.file_manager.load_scene import load_scene
from scripts.file_manager.get_file import save_file_selector, load_file_selector
import platform


class InputHandler:
    def __init__(self, editor) -> None:
        # Reference to the editor
        self.editor = editor
        self.dim =    editor.viewport_dim
        
        # Input attributes
        self.mouse_viewport = None
        """The instance of the viewport that the mouse is currently in."""
        self.mouse_viewport_key: str = None
        """Name of the current mouse viewport. Used for comparisons"""
        self.mouse_lock_position: tuple = (0, 0)
        """Position of the mouse before it is locked (used to restore position when unlocked)"""
        self.camera_active: bool = False
        """Bool determining if the editor camera is being used by the user"""
        self.dragging: str = ''
        """The viewport border name that is currently being dragged. Empty string if none"""
        self.selected_attrib = None
        """The attribute that is being edited"""
        self.input_string: str = ''
        """Current value of the text input string"""
        self.input_viewport: str = 'inspector'

    def get_viewports(self):
        self.viewports = {
            'toolbar' : self.editor.ui.toolbar,
            'files' : self.editor.ui.project_files_view,
            'hierarchy' : self.editor.ui.hierarchy,
            'inspector' : self.editor.ui.inspector,
            'viewport' : self.editor.ui.viewport
        }

    def update(self):
        """
        
        """
        
        # Get all input lists from the engine
        keys      = self.editor.engine.keys
        prev_keys = self.editor.engine.prev_keys
        mouse_x, mouse_y   = self.editor.engine.mouse_position
        mouse_buttons      = self.editor.engine.mouse_buttons
        prev_mouse_buttons = self.editor.engine.prev_mouse_buttons
        win_size = self.editor.engine.win_size

        # Get the current mouse viewport
        if   mouse_y < (    self.dim.top   ) * win_size[1]: self.mouse_viewport_key = 'toolbar'
        elif mouse_y > (1 - self.dim.bottom) * win_size[1]: self.mouse_viewport_key = 'files'
        elif mouse_x < (    self.dim.left  ) * win_size[0]: self.mouse_viewport_key = 'hierarchy'
        elif mouse_x > (1 - self.dim.right ) * win_size[0]: self.mouse_viewport_key = 'inspector'
        else:                                               self.mouse_viewport_key = 'viewport'

        # Loop through all input events
        for event in self.editor.engine.events:
            if event.type == pg.MOUSEWHEEL:
                # Mouse has been scrolled, send input to mouse's viewport
                self.viewports[self.mouse_viewport_key].scroll(event.y)

            if event.type == pg.KEYDOWN and self.selected_attrib:
                # Key has been pressed and the user is using a text box

                if event.key == pg.K_BACKSPACE: self.input_string = self.input_string[:-1]
                elif event.key == pg.K_RETURN: self.viewports[self.input_viewport].apply_input_string()
                elif event.key == pg.K_ESCAPE: 
                    self.input_string = ''
                    self.selected_attrib = None
                else: self.input_string += event.unicode
                self.editor.ui.refresh()

        # Check for viewport dragging
        if mouse_buttons[0]:
            if not self.dragging:
                threshold = 5  # Number of pixels in each direction of a border that a drag can start
                if (1 - self.dim.bottom) * win_size[1] - threshold < mouse_y <= (1 - self.dim.bottom) * win_size[1] + threshold:
                    self.dragging = 'bottom'
                elif self.dim.left * win_size[0] - threshold < mouse_x <= self.dim.left * win_size[0] + threshold and self.dim.top * win_size[1] < mouse_y <= (1 - self.dim.bottom) * win_size[1]:
                    self.dragging = 'left'
                elif (1 - self.dim.right) * win_size[0] - threshold < mouse_x <= (1 - self.dim.right) * win_size[0] + threshold and self.dim.top * win_size[1] < mouse_y <= (1 - self.dim.bottom) * win_size[1]:
                    self.dragging = 'right'
                
            if not self.dragging and self.mouse_viewport_key == 'viewport':
                x = (mouse_x / win_size[0] - self.dim.left) / (1 - self.dim.left - self.dim.right)  * win_size[0]
                y = (mouse_y / win_size[1] - self.dim.top ) / (1 - self.dim.top  - self.dim.bottom) * win_size[1]
                node = self.editor.engine.project.current_scene.get_model_node_at(x, y)
                if node: 
                    nodes = self.editor.engine.project.current_scene.node_handler.nodes
                    index = nodes.index(node)
                    self.editor.ui.hierarchy.selected_node_index = index
                    self.editor.ui.refresh()
                else:
                    self.editor.ui.hierarchy.selected_node_index = -1
                    self.editor.ui.refresh()
        elif prev_mouse_buttons[0]:
            self.dragging = ''
            self.input_string = ''
            self.selected_text_attrib = None

        # Delete button
        if prev_keys[pg.K_DELETE] and not keys[pg.K_DELETE] or prev_keys[pg.K_BACKSPACE] and not keys[pg.K_BACKSPACE]:
            node_handler = self.editor.engine.project.current_scene.node_handler
            if self.editor.ui.hierarchy.selected_node_index < 0: return
            node = node_handler.nodes[self.editor.ui.hierarchy.selected_node_index]
            if not node: return
            node_handler.remove(node)
            self.editor.ui.refresh()

        match self.dragging:
            case 'bottom':
                self.dim.bottom = min(max(1 - mouse_y / win_size[1], 0.001), 0.9)
            case 'left':
                self.dim.left   = min(max(mouse_x / win_size[0], 0), .95 - self.dim.right)
            case 'right':
                self.dim.right  = min(max(1 - mouse_x / win_size[0], 0), .95 - self.dim.left)

        # Save and load scenes
        if keys[pg.K_LCTRL] and keys[pg.K_s]:
            if platform.system() == "Darwin": 
                file = input("Enter file path: ")
                if not file.endswith('.gltf'): file += ".gltf"
            else: file = save_file_selector()
            if file: 
                # Get the scene
                scene = self.editor.engine.project.current_scene
                # Save
                save_scene(scene, abs_file_path=file)

        if keys[pg.K_LCTRL] and keys[pg.K_l]:
            if platform.system() == "Darwin": 
                file = input("Enter file path: ")
                if not file.endswith('.gltf'): file += ".gltf"
            else: file = load_file_selector()
            if file:
                # Get the scene
                scene = self.editor.engine.project.current_scene
                # Load and update scene
                load_scene(scene, abs_file_path=file)
                scene.vao_handler.shader_handler.write_all_uniforms()
                scene.project.texture_handler.write_textures()
                scene.project.texture_handler.write_textures('batch')
                scene.light_handler.write('batch')
                scene.material_handler.write('batch')
                # Refresh the UI
                self.editor.ui.generate_vbo_images()
                self.editor.ui.refresh()

        # Right click
        if mouse_buttons[2] and prev_mouse_buttons[2]: 
            ...
        elif mouse_buttons[2] and self.mouse_viewport_key == 'viewport':
            self.camera_active = True
            pg.mouse.get_rel()
            # Lock mouse
            self.mouse_lock_position = (mouse_x, mouse_y)
            pg.event.set_grab(True)
            pg.mouse.set_visible(False)
        elif prev_mouse_buttons[2]:
            # Unlock mouse
            pg.event.set_grab(False)
            pg.mouse.set_visible(True)
            if self.camera_active: pg.mouse.set_pos(self.mouse_lock_position)

            self.camera_active = False
        
        
        if not self.camera_active and not self.dragging: self.viewports[self.mouse_viewport_key].get_input()

        if self.camera_active: self.editor.camera.update()