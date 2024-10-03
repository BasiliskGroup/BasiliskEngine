import pygame as pg


class ProjectFilesView:
    def __init__(self, editor) -> None:
        # Reference to enviornment
        self.editor       = editor
        self.engine       = self.editor.engine
        self.viewport_dim = editor.viewport_dim

        self.set_surf()  # Set the surface for drawing

        # Display attributes
        self.padding = 3
        self.grid_x, self.grid_y = 8, 3
        self.list_item_height = 30
        self.menu = "Models"


    def render(self):
        """Clears the viewport and renders all elements"""
        self.surf.fill(self.editor.ui.primary)

        pg.draw.rect(self.surf, self.editor.ui.secondary, (0, 0, self.editor.viewport_dim.left * self.engine.win_size[0], self.dim[1]))

        if self.menu == "Models"  : pg.draw.rect(self.surf, self.editor.ui.accent, (0, self.list_item_height * 0, self.editor.viewport_dim.left * self.engine.win_size[0], self.list_item_height))
        self.editor.font.render_text(self.surf, (self.padding + 15, self.list_item_height * 0.5), "Models"  , size=0)
        if self.menu == "Textures": pg.draw.rect(self.surf, self.editor.ui.accent, (0, self.list_item_height * 1, self.editor.viewport_dim.left * self.engine.win_size[0], self.list_item_height))
        self.editor.font.render_text(self.surf, (self.padding + 15, self.list_item_height * 1.5), "Textures", size=0)
        if self.menu == "Scripts" : pg.draw.rect(self.surf, self.editor.ui.accent, (0, self.list_item_height * 2, self.editor.viewport_dim.left * self.engine.win_size[0], self.list_item_height))
        self.editor.font.render_text(self.surf, (self.padding + 15, self.list_item_height * 2.5), "Scripts" , size=0)
        if self.menu == "Shaders" : pg.draw.rect(self.surf, self.editor.ui.accent, (0, self.list_item_height * 3, self.editor.viewport_dim.left * self.engine.win_size[0], self.list_item_height))
        self.editor.font.render_text(self.surf, (self.padding + 15, self.list_item_height * 3.5), "Shaders" , size=0)

        if self.menu == "Models":
            self.render_image_grid(self.editor.ui.model_images)
        if self.menu == "Textures":
            self.render_image_grid(self.engine.project.texture_handler.texture_surfaces.values())

        pg.draw.rect(self.surf, self.editor.ui.outline, (0, 0, self.dim[0], self.dim[1]), 1)

    def render_image_grid(self, images, size=100):
        left = self.editor.viewport_dim.left * self.engine.win_size[0]
        width, height = size, size
        self.grid_y = self.dim[1] // height
        self.grid_x = (self.dim[0] - left) // width

        for i, image in enumerate(images):
            x, y = i % self.grid_x, i // self.grid_x
            image = pg.transform.scale(image, (width - self.padding * 2, height - self.padding * 2))
            self.surf.blit(image, (left + x * width + self.padding, y * height + self.padding))

    def get_input(self) -> None:
        if self.engine.prev_mouse_buttons[0] and not self.engine.mouse_buttons[0]:
            # Get the mouse position in the window
            mouse_x, mouse_y = self.engine.mouse_position[0], self.engine.mouse_position[1] - (1 - self.editor.viewport_dim.bottom) * self.engine.win_size[1]
            left = self.editor.viewport_dim.left * self.engine.win_size[0]

            # In the menu side bar, selects what this window will show
            if mouse_x < left:
                y = int(mouse_y // self.list_item_height)

                if y <= 3: self.menu = ["Models", "Textures", "Scripts", "Shaders"][y]

                self.editor.ui.refresh()

            # In the list section, get the selected list item
            else:
                left = self.editor.viewport_dim.left * self.engine.win_size[0]
                width, height = (self.dim[0] - left - self.padding * 2) // self.grid_x, (self.dim[1] - self.padding * 2) // self.grid_y
                x, y = (mouse_x - left) // width, mouse_y // height

                index = int(x + y * self.grid_x)

                # Adds a model to the scene
                if self.menu == "Models" and index < len(self.engine.project.current_scene.vao_handler.vbo_handler.vbos.keys()):
                    model = list(self.engine.project.current_scene.vao_handler.vbo_handler.vbos.keys())[index]
                    self.engine.project.current_scene.node_handler.add(model=model, name=model)
                    self.editor.ui.refresh()

                elif self.menu == "Textures" and index < len(self.engine.project.current_scene.vao_handler.vbo_handler.vbos.keys()):
                    texture = list(self.engine.project.texture_handler.textures.keys())[index]
                    texture = self.engine.project.texture_handler.texture_ids[texture]
                    objects = self.engine.project.current_scene.object_handler.objects
                    object_index = self.editor.ui.hierarchy.selected_object_index
                    if 0 <= object_index < len(objects):
                        objects[object_index].texture = texture
                        self.editor.ui.refresh()
                    


    def scroll(self, value) -> None:
        ...

    def set_surf(self) -> None:
        """Sets the viewport surface for drawing onto."""
        self.dim = self.viewport_dim.get_project_files_pixels(self.engine.win_size)
        self.surf = pg.Surface(self.dim).convert_alpha()