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
        self.objects = self.engine.project.current_scene.node_handler.nodes
        self.selected_object = None

        # Display attributes
        self.item_height = 30
        self.padding = 3
        self.indent_pixels = 10

        self.set_surf()  # Set the surface for drawing

    def render(self):
        """Clears the viewport and renders all elements"""

        # Clear the screen
        self.surf.fill(self.editor.ui.primary)

        # Get the object selected in the hierarchy
        self.get_object()

        # Reset the component height to the top
        self.component_height = 0
        # Reset the current attribute boxes (like buttons)
        self.attribute_boxes = []

        if self.selected_object: self.render_title()
        # if self.selected_object: self.render_transform_component()


        pg.draw.rect(self.surf, self.editor.ui.outline, (0, 0, self.dim[0], self.dim[1]), 1)

    def render_title(self):
        """
        Renders the inspector title (usually the object name)
        """
        
        # Draw background box
        pg.draw.rect(self.surf, self.ui.accent, (0, 0, self.dim[0], self.item_height * 2))
        # Draw the title
        self.font.render_text(self.surf, (self.dim[0] / 2, self.item_height), self.selected_object.name, size=1, center_width=True)
        
        # Update the component height
        self.component_height += self.item_height * 2

    def render_transform_component(self):
        start_x, start_y = 45, self.top_buffer + self.component_height
        w, h = (self.dim[0] - start_x - 15) // 3, self.list_item_height
        padding = self.padding
        obj = self.selected_object
        size = w - padding * 2, h - padding * 2
        # Position
        self.editor.font.render_text(self.surf, (padding, start_y + h * 0 + h/2), 'Pos', size=0)
        self.attribute_boxes.append((obj.position, (start_x + w * 0 + padding, start_y + h * 0 + padding, *size), 'x'))
        self.attribute_boxes.append((obj.position, (start_x + w * 1 + padding, start_y + h * 0 + padding, *size), 'y'))
        self.attribute_boxes.append((obj.position, (start_x + w * 2 + padding, start_y + h * 0 + padding, *size), 'z'))
        # Rotation
        self.editor.font.render_text(self.surf, (padding, start_y + h * 1 + h/2), 'Rot', size=0)
        self.attribute_boxes.append((obj.rotation, (start_x + w * 0 + padding, start_y + h * 1 + padding, *size), 'x'))
        self.attribute_boxes.append((obj.rotation, (start_x + w * 1 + padding, start_y + h * 1 + padding, *size), 'y'))
        self.attribute_boxes.append((obj.rotation, (start_x + w * 2 + padding, start_y + h * 1 + padding, *size), 'z'))
        # Scale
        self.editor.font.render_text(self.surf, (padding, start_y + h * 2 + h/2), 'Scale', size=0)
        self.attribute_boxes.append((obj.scale   , (start_x + w * 0 + padding, start_y + h * 2 + padding, *size), 'x'))
        self.attribute_boxes.append((obj.scale   , (start_x + w * 1 + padding, start_y + h * 2 + padding, *size), 'y'))
        self.attribute_boxes.append((obj.scale   , (start_x + w * 2 + padding, start_y + h * 2 + padding, *size), 'z'))

        self.component_height += h * 4

    def render_attribute_box(self, object, rect, attrib_name):
        """
        Renders a single attribute box.
        """
        
        # Check if the attribute in the box is being edited by the user
        if self.input.selected_text_attrib == attrib_name: 
            color = self.ui.accent
            text = self.input.input_string
        else: 
            color = self.ui.secondary
            value = getattr(object, attrib_name)
            text = f'{value:.2f}'
        pg.draw.rect(self.surf, color, rect)
        self.editor.font.render_text(self.surf, (rect[0] + rect[2] / 2, rect[1] + rect[3] / 2), text, size=3, center_width=True)

    def get_input(self) -> None:
        ...

    def scroll(self, value) -> None:
        ...

    def get_object(self):
        """
        Gets the object currently being selected by the hierarchy
        """
        
        # Check that the object index is pointing to a real object
        if 0 <= self.ui.hierarchy.selected_object_index < len(self.objects):
            self.selected_object = self.objects[self.ui.hierarchy.selected_object_index]
        else:
            self.selected_object = None

        return self.selected_object

    def set_surf(self) -> None:
        """Sets the viewport surface for drawing onto."""
        self.dim = self.viewport_dim.get_inspector_pixels(self.engine.win_size)
        self.surf = pg.Surface(self.dim).convert_alpha()