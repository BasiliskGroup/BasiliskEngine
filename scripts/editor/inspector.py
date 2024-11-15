import pygame as pg
import glm


class Inspector:
    def __init__(self, editor, ui) -> None:
        # Reference to enviornment
        self.editor       = editor
        self.engine       = self.editor.engine
        self.input        = editor.input
        self.ui           = ui
        self.font         = editor.font
        self.viewport_dim = editor.viewport_dim

        # Selection attributes
        self.scroll_value = 0
        self.component_height = 0
        self.nodes = self.engine.project.current_scene.node_handler.nodes
        self.selected_node = None

        # Display attributes
        self.item_height = int(30 * self.editor.window_scale)
        self.padding = int(3 * self.editor.window_scale)
        self.indent_pixels = int(10 * self.editor.window_scale)

        self.set_surf()  # Set the surface for drawing

    def render(self):
        """Clears the viewport and renders all elements"""

        # Clear the screen
        self.surf.fill(self.editor.ui.primary)

        # Get the node selected in the hierarchy
        self.get_node()

        # Reset the component height to the top
        self.component_height = 0
        # Reset the current attribute boxes (like buttons)
        self.attribute_boxes = []
        self.toggles = []

        if self.selected_node: 
            self.render_title()
            self.render_transform_component()
            self.render_mtl_component()
            self.render_physics_component()
            self.render_tags_component()

        for box in self.attribute_boxes:
            self.render_attribute_box(*box)
        for toggle in self.toggles:
            self.render_toggle(*toggle)

        pg.draw.rect(self.surf, self.editor.ui.outline, (0, 0, self.dim[0], self.dim[1]), 1)

    def render_title(self):
        """
        Renders the inspector title (usually the node name)
        """
        
        # Draw background box
        pg.draw.rect(self.surf, self.ui.accent, (0, 0, self.dim[0], self.item_height * 2))
        # Draw the title
        self.font.render_text(self.surf, (self.dim[0] / 2, self.item_height), self.selected_node.name, size=1, center_width=True)
        
        # Update the component height
        self.component_height += self.item_height * 2

    def render_transform_component(self):
        start_x, start_y = 65, self.component_height
        w, h = (self.dim[0] - start_x - 15) // 3, self.item_height
        padding = self.padding
        node = self.selected_node
        size = w - padding * 2, h - padding * 2
        # Position
        self.editor.font.render_text(self.surf, (padding, start_y + h * 0 + h/2), 'Pos', size=0)
        self.attribute_boxes.append((node.position, (start_x + w * 0 + padding, start_y + h * 0 + padding, *size), 'x'))
        self.attribute_boxes.append((node.position, (start_x + w * 1 + padding, start_y + h * 0 + padding, *size), 'y'))
        self.attribute_boxes.append((node.position, (start_x + w * 2 + padding, start_y + h * 0 + padding, *size), 'z'))
        # Rotation
        self.editor.font.render_text(self.surf, (padding, start_y + h * 1 + h/2), 'Rot', size=0)
        self.attribute_boxes.append((node.manual_rotation, (start_x + w * 0 + padding, start_y + h * 1 + padding, *size), 'x'))
        self.attribute_boxes.append((node.manual_rotation, (start_x + w * 1 + padding, start_y + h * 1 + padding, *size), 'y'))
        self.attribute_boxes.append((node.manual_rotation, (start_x + w * 2 + padding, start_y + h * 1 + padding, *size), 'z'))
        # Scale
        self.editor.font.render_text(self.surf, (padding, start_y + h * 2 + h/2), 'Scale', size=0)
        self.attribute_boxes.append((node.scale   , (start_x + w * 0 + padding, start_y + h * 2 + padding, *size), 'x'))
        self.attribute_boxes.append((node.scale   , (start_x + w * 1 + padding, start_y + h * 2 + padding, *size), 'y'))
        self.attribute_boxes.append((node.scale   , (start_x + w * 2 + padding, start_y + h * 2 + padding, *size), 'z'))

        self.component_height += h * 4

    def render_mtl_component(self):
        start_x, start_y = 65, self.component_height
        w, h = (self.dim[0] - start_x - 15) // 3, self.item_height
        padding = self.padding
        node = self.selected_node
        size = w - padding * 2, h - padding * 2

        if not node.model: return

        mtl_handler = self.engine.project.current_scene.material_handler
        for mtl_name in mtl_handler.material_ids:
            if node.model.material != mtl_handler.material_ids[mtl_name]: continue
            mtl = mtl_handler.materials[mtl_name]
            break

        # Color
        self.editor.font.render_text(self.surf, (padding, start_y + h * 0 + h/2), 'Color', size=0)
        self.attribute_boxes.append((mtl.color, (start_x + w * 0 + padding, start_y + h * 0 + padding, *size), 'x'))
        self.attribute_boxes.append((mtl.color, (start_x + w * 1 + padding, start_y + h * 0 + padding, *size), 'y'))
        self.attribute_boxes.append((mtl.color, (start_x + w * 2 + padding, start_y + h * 0 + padding, *size), 'z'))
        
        # Specular
        self.editor.font.render_text(self.surf, (padding, start_y + h * 1 + h/2), 'Spec', size=0)
        self.attribute_boxes.append((mtl.specular, (start_x + w * 0 + padding, start_y + h * 1 + padding, size[0] * 1.5 + padding, size[1]), 'value'))
        self.attribute_boxes.append((mtl.specular_exponent, (start_x + w * 1.5 + padding, start_y + h * 1 + padding, size[0] * 1.5 + padding, size[1]), 'value'))
       
        # Alpha
        self.editor.font.render_text(self.surf, (padding, start_y + h * 2 + h/2), 'Alpha', size=0)
        self.attribute_boxes.append((mtl.alpha, (start_x + w * 0 + padding, start_y + h * 2 + padding, size[0] * 3 + padding * 4, size[1]), 'value'))
        
        # Albedo/Normal Map
        self.editor.font.render_text(self.surf, (    self.dim[0] / 4, start_y + h * 3 + h/2), 'Texture', size=0, center_width=True)
        self.editor.font.render_text(self.surf, (3 * self.dim[0] / 4, start_y + h * 3 + h/2), 'Normal',  size=0, center_width=True)

        img_padding = 3
        img_size = max(min(self.dim[0] / 2 - img_padding * 2, 100), 1)

        if mtl.texture:
            img = self.engine.project.texture_handler.texture_surfaces[mtl.texture]
            img = pg.transform.scale(img, (img_size, img_size))
            self.surf.blit(img, ((self.dim[0]/2 - img_size) / 2, start_y + h * 4))
        else:
            color = (max(min(mtl.color[0] * 255, 255), 0), max(min(mtl.color[1] * 255, 255), 0), max(min(mtl.color[2] * 255, 255), 0))
            pg.draw.rect(self.surf, color, ((self.dim[0]/2 - img_size) / 2, start_y + h * 4, img_size, img_size))

        if mtl.normal_map:
            img = self.engine.project.texture_handler.texture_surfaces[mtl.normal_map]
            img = pg.transform.scale(img, (img_size, img_size))
            self.surf.blit(img, (self.dim[0]/2 + (self.dim[0]/2 - img_size) / 2, start_y + h * 4))
        else:
            pg.draw.rect(self.surf, (0, 0, 0), (self.dim[0]/2 + (self.dim[0]/2 - img_size) / 2, start_y + h * 4, img_size, img_size), 2)

        self.component_height += h * 4 + img_size + padding

    def render_physics_component(self):
        start_x, start_y = 65, self.component_height
        w, h = (self.dim[0] - start_x - 15) // 3, self.item_height
        padding = self.padding
        node = self.selected_node
        size = w - padding * 2, h - padding * 2

        y_level = start_y

        # Has Collider body
        self.editor.font.render_text(self.surf, (padding, y_level + h/2), 'Collider', size=0)
        self.toggles.append((node, (start_x + w * 0 + padding, y_level + padding, h - padding * 2, h - padding * 2), 'collider'))
        y_level += h

        # Is static
        if node.collider:
            self.editor.font.render_text(self.surf, (padding, y_level + h/2), 'Static', size=0)
            self.toggles.append((node.collider, (start_x + w * 0 + padding, y_level + padding, h - padding * 2, h - padding * 2), 'static'))
            y_level += h

        # Has Physics body
        self.editor.font.render_text(self.surf, (padding, y_level + h/2), 'Physics', size=0)
        self.toggles.append((node, (start_x + w * 0 + padding, y_level + padding, h - padding * 2, h - padding * 2), 'physics_body'))
        y_level += h

        self.component_height += h * 3

        # Mass
        if node.physics_body:
            start_x, start_y = 65, self.component_height
            w, h = (self.dim[0] - start_x - 15) // 3, self.item_height
            padding = self.padding
            node = self.selected_node
            size = w - padding * 2, h - padding * 2

            self.editor.font.render_text(self.surf, (padding, y_level + h/2), 'Mass', size=0)
            self.attribute_boxes.append((node.physics_body, (start_x + w * 0 + padding, y_level + padding, size[0] * 3 + padding * 4, size[1]), 'mass'))
            y_level += h
            self.component_height += h

    def render_tags_component(self):
        start_x, start_y = 65, self.component_height
        w, h = (self.dim[0] - start_x - 15) // 3, self.item_height
        padding = self.padding
        node = self.selected_node
        size = w - padding * 2, h - padding * 2

        self.editor.font.render_text(self.surf, (padding, start_y + h * 2 + h/2), 'Tags', size=0)
        self.attribute_boxes.append((node, (start_x + w * 0 + padding, start_y + h * 2 + padding, size[0] * 3 + padding * 4, size[1]), 'tags'))

    def render_attribute_box(self, node, rect, attrib_name):
        """
        Renders a single attribute box.
        """
        
        # Check if the attribute in the box is being edited by the user
        if self.input.selected_attrib == (node, attrib_name): 
            color = self.ui.accent
            text = self.input.input_string
        else: 
            color = self.ui.secondary
            value = getattr(node, attrib_name)
            try:
                float(value)
                text = f'{value:.2f}'
            except ValueError:
                text = value
        pg.draw.rect(self.surf, color, rect)
        self.editor.font.render_text(self.surf, (rect[0] + rect[2] / 2, rect[1] + rect[3] / 2), text, size=3, center_width=True)

    def render_toggle(self, node, rect, attrib_name):
        """
        Renders a toggle button for a boolean value
        """
        
        width = 3
        pg.draw.rect(self.surf, (159, 150, 150) if getattr(node, attrib_name) else self.ui.accent, rect, width)
        pg.draw.rect(self.surf, (0, 0, 0), rect, 1)
        if getattr(node, attrib_name): 
            pg.draw.rect(self.surf, (159, 150, 150), (rect[0] + width + 2, rect[1] + width + 2, rect[2] - width * 2 - 4, rect[3] - width * 2 - 4))

    def get_node(self):
        """
        Gets the node currently being selected by the hierarchy
        """
        
        # Check that the node index is pointing to a real node
        if 0 <= self.ui.hierarchy.selected_node_index < len(self.nodes):
            self.selected_node = self.nodes[self.ui.hierarchy.selected_node_index]
        else:
            self.selected_node = None

        return self.selected_node

    def get_input(self) -> None:
        mouse_x, mouse_y = self.engine.mouse_position[0] - (1 - self.editor.viewport_dim.right) * self.engine.win_size[0], self.engine.mouse_position[1] - self.editor.viewport_dim.top * self.engine.win_size[1]
        
        if self.engine.prev_mouse_buttons[0] and not self.engine.mouse_buttons[0]:
            # Check for input into an attribute box
            for box in self.attribute_boxes:
                box_pos = box[1]
                if box_pos[0] < mouse_x <= box_pos[0] + box_pos[2] and box_pos[1] < mouse_y <= box_pos[1] + box_pos[3]:
                    self.input.selected_attrib = (box[0], box[2])
                    self.ui.refresh()
            # Check if a toggle should be switched
            for toggle in self.toggles:
                toggle_pos = toggle[1]
                if toggle_pos[0] < mouse_x <= toggle_pos[0] + toggle_pos[2] and toggle_pos[1] < mouse_y <= toggle_pos[1] + toggle_pos[3]:
                    self.apply_toggle(toggle[0], toggle[2])
                    self.ui.refresh()

    def apply_input_string(self):

        try:
            input_value = float(self.input.input_string)
        except:
            self.input.selected_attrib = (None, None)
            self.input.input_string = ''
            return

        obj = self.input.selected_attrib[0]
        attrib = self.input.selected_attrib[1]
        setattr(obj, attrib, input_value)
        self.input.selected_attrib = (None, None)
        self.input.input_string = ''

        self.editor.viewport_resize()

    def apply_toggle(self, obj, attrib):
        scene = self.engine.project.current_scene
        match attrib:
            case "physics_body":
                if obj.physics_body:
                    obj.physics_body = None
                else:
                    physics_body = scene.physics_body_handler.add(mass=1, rotation=glm.quat(obj.rotation))
                    obj.physics_body = physics_body
            case "collider":
                if obj.collider:
                    obj.collider = None
                else:
                    print(obj.model.vbo)
                    collider = scene.collider_handler.add(vbo=obj.model.vbo, static=True)
                    obj.collider = collider
            case "static":
                obj.static = not obj.static

    def scroll(self, value) -> None:
        ...

    def set_surf(self) -> None:
        """Sets the viewport surface for drawing onto."""
        win_size = self.engine.win_size[0] * self.editor.window_scale, self.engine.win_size[1] * self.editor.window_scale
        self.dim = self.viewport_dim.get_inspector_pixels(win_size)
        self.surf = pg.Surface(self.dim).convert_alpha()