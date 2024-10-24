import pygame as pg


class ProjectFilesView:
    def __init__(self, editor) -> None:
        # Reference to enviornment
        self.editor       = editor
        self.engine       = self.editor.engine
        self.viewport_dim = editor.viewport_dim

        self.set_surf()  # Set the surface for drawing

        self.nodes = self.engine.project.current_scene.node_handler.nodes
        self.textures = self.engine.project.texture_handler.textures
        self.mtl_handler = self.engine.project.current_scene.material_handler

        # Display attributes
        self.padding = int(3 * self.editor.window_scale)
        self.grid_x, self.grid_y = int(8 * self.editor.window_scale), int(3 * self.editor.window_scale)
        self.list_item_height = int(30 * self.editor.window_scale)
        self.image_size = int(100 * self.editor.window_scale)
        self.menu = "Models"


    def render(self):
        """Clears the viewport and renders all elements"""
        self.surf.fill(self.editor.ui.primary)

        pg.draw.rect(self.surf, self.editor.ui.secondary, (0, 0, self.editor.viewport_dim.left * self.engine.win_size[0] * self.editor.window_scale, self.dim[1]))

        self.get_node()

        if self.menu == "Models"  : pg.draw.rect(self.surf, self.editor.ui.accent, (0, self.list_item_height * 0, self.editor.viewport_dim.left * self.engine.win_size[0], self.list_item_height))
        self.editor.font.render_text(self.surf, (self.padding + 15, self.list_item_height * 0.5), "Models"  , size=0)
        if self.menu == "Textures": pg.draw.rect(self.surf, self.editor.ui.accent, (0, self.list_item_height * 1, self.editor.viewport_dim.left * self.engine.win_size[0], self.list_item_height))
        self.editor.font.render_text(self.surf, (self.padding + 15, self.list_item_height * 1.5), "Textures", size=0)
        if self.menu == "Materials" : pg.draw.rect(self.surf, self.editor.ui.accent, (0, self.list_item_height * 2, self.editor.viewport_dim.left * self.engine.win_size[0], self.list_item_height))
        self.editor.font.render_text(self.surf, (self.padding + 15, self.list_item_height * 2.5), "Materials" , size=0)
        if self.menu == "Shaders" : pg.draw.rect(self.surf, self.editor.ui.accent, (0, self.list_item_height * 3, self.editor.viewport_dim.left * self.engine.win_size[0], self.list_item_height))
        self.editor.font.render_text(self.surf, (self.padding + 15, self.list_item_height * 3.5), "Shaders" , size=0)

        if self.menu == "Models":
            self.render_image_grid(self.editor.ui.model_images)
        if self.menu == "Textures":
            self.render_image_grid(self.engine.project.texture_handler.texture_surfaces.values())
        if self.menu == "Materials":
            self.render_mtls()

        pg.draw.rect(self.surf, self.editor.ui.outline, (0, 0, self.dim[0], self.dim[1]), 1)

    def render_image_grid(self, images):
        left = self.editor.viewport_dim.left * self.engine.win_size[0] * self.editor.window_scale
        width, height = self.image_size, self.image_size
        self.grid_y = self.dim[1] // height
        self.grid_x = (self.dim[0] - left) // width

        for i, image in enumerate(images):
            x, y = i % self.grid_x, i // self.grid_x
            image = pg.transform.scale(image, (width - self.padding * 2, height - self.padding * 2))
            self.surf.blit(image, (left + x * width + self.padding, y * height + self.padding))

    def render_mtls(self):
        images = []
        for mtl in self.mtl_handler.materials.values():
            if mtl.texture:
                img = self.engine.project.texture_handler.texture_surfaces[mtl.texture]
                img = pg.transform.scale(img, (self.image_size, self.image_size))
            else:
                img = pg.Surface((self.image_size, self.image_size)).convert()
                color = (min(mtl.color.r * 255, 255), min(mtl.color.g * 255, 255), min(mtl.color.b * 255, 255))
                img.fill(color)

            images.append(img)
        self.render_image_grid(images)

    def get_input(self) -> None:
        if self.engine.prev_mouse_buttons[0] and not self.engine.mouse_buttons[0]:
            # Get the mouse position in the window
            mouse_x, mouse_y = self.engine.mouse_position[0], self.engine.mouse_position[1] - (1 - self.editor.viewport_dim.bottom) * self.engine.win_size[1] * self.editor.window_scale
            left = self.editor.viewport_dim.left * self.engine.win_size[0] * self.editor.window_scale

            # In the menu side bar, selects what this window will show
            if mouse_x < left:
                y = int(mouse_y // self.list_item_height)

                if y <= 3: self.menu = ["Models", "Textures", "Materials", "Shaders"][y]

                self.editor.ui.refresh()

            # In the list section, get the selected list item
            else:
                left = self.editor.viewport_dim.left * self.engine.win_size[0] * self.editor.window_scale
                x, y = (mouse_x - left) // self.image_size, mouse_y // self.image_size

                index = int(x + y * self.grid_x)

                # Adds a model to the scene
                if self.menu == "Models" and index < len(self.engine.project.current_scene.vao_handler.vbo_handler.vbos.keys()):
                    model = list(self.engine.project.current_scene.vao_handler.vbo_handler.vbos.keys())[index]
                    self.engine.project.current_scene.node_handler.add(model=model, name=model)
                    self.editor.ui.refresh()

                elif self.menu == "Textures":
                    node = self.selected_node
                    if not node or not node.model: return

                    for mtl_name in self.mtl_handler.material_ids:
                        if node.model.material != self.mtl_handler.material_ids[mtl_name]: continue
                        mtl = self.mtl_handler.materials[mtl_name]
                        break
                    
                    if index < len(self.textures):
                        texture = list(self.textures.keys())[index]

                        if self.engine.keys[pg.K_LSHIFT]:
                            mtl.normal_map = texture
                        else:
                            mtl.texture = texture
                    else:
                        if self.engine.keys[pg.K_LSHIFT]:
                            mtl.normal_map = None
                        else:
                            mtl.texture = None
                    
                    self.editor.ui.refresh()
                    self.editor.viewport_resize()

                elif self.menu == "Materials":
                    if index < len(self.mtl_handler.materials):
                        mtl = list(self.mtl_handler.materials.keys())[index]

                        node = self.selected_node
                        if not node or not node.model: return

                        node.model.material = mtl
                    else:
                        self.mtl_handler.add(name=f"Material_{len(self.mtl_handler.materials) + 1}")
                    
                    self.editor.ui.refresh()

    def get_node(self):
        """
        Gets the node currently being selected by the hierarchy
        """
        
        # Check that the node index is pointing to a real node
        if 0 <= self.editor.ui.hierarchy.selected_node_index < len(self.nodes):
            self.selected_node = self.nodes[self.editor.ui.hierarchy.selected_node_index]
        else:
            self.selected_node = None

        return self.selected_node

    def scroll(self, value) -> None:
        if self.engine.keys[pg.K_LCTRL]:
            self.image_size = max(self.image_size + value * 10, 10)
        self.editor.ui.refresh()

    def set_surf(self) -> None:
        """Sets the viewport surface for drawing onto."""
        win_size = self.engine.win_size[0] * self.editor.window_scale, self.engine.win_size[1] * self.editor.window_scale
        self.dim = self.viewport_dim.get_project_files_pixels(win_size)
        self.surf = pg.Surface(self.dim).convert_alpha()