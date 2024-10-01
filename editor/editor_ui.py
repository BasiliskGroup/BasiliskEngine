import pygame as pg
import moderngl as mgl
import glm
from scripts.editor.viewport import Viewport
from scripts.editor.hierarchy import Hierarchy
from scripts.editor.inspector import Inspector
from scripts.editor.toolbar import Toolbar
from scripts.editor.project_view import ProjectFilesView


class EditorUI:
    def __init__(self, editor) -> None:
        # Reference to the enviornment
        self.editor  = editor
        self.ctx     = editor.ctx
        self.vao     = editor.vao
        self.program = editor.program
        self.texture = None

        # Surface for blitting viewports and a texture for rendering UI
        self.set_surface_texture()

        self.do_update_texture = True

        # Colors for drawing :)
        self.primary    = pg.Color(40 , 40 , 40 , 255)
        self.secondary  = pg.Color(50 , 50 , 50 , 255)
        self.accent     = pg.Color(60 , 60 , 60 , 255)
        self.outline    = pg.Color(10 , 10 , 10 , 255)
        self.text_color = pg.Color(255, 255, 255, 255)

        # Load assets
        self.logo_source = pg.image.load('scripts/editor/editor_assets/BailiskLogoWhite.png').convert_alpha()
        self.logo = pg.transform.scale(self.logo_source, 
                                       (self.editor.viewport_dim.top * self.editor.engine.win_size[1] * .8, 
                                        self.editor.viewport_dim.top * self.editor.engine.win_size[1] * .8))

        # Viewports/Windows
        self.viewport           = Viewport(editor)
        self.hierarchy          = Hierarchy(editor)
        self.inspector          = Inspector(editor, self)
        self.toolbar            = Toolbar(editor)
        self.project_files_view = ProjectFilesView(editor)

    def render(self):
        """
        Renders all the viewports of the editor UI. Renders the engine framebuffer.
        """
        
        if self.do_update_texture:
            # Clears the screem
            self.surf.fill((0, 0, 0, 0))

            # Renders the individual viewport/window surfaces
            self.render_viewports()

            # Gets enviornment variables
            win_size = self.editor.engine.win_size
            dim = self.editor.viewport_dim

            # Blits all viewport windows to the UI surf
            self.surf.blit(self.toolbar.surf, (0, 0))
            self.surf.blit(self.hierarchy.surf, (0, win_size[1] * dim.top))
            self.surf.blit(self.inspector.surf, (win_size[0] * (1 - dim.right), win_size[1] * dim.top))
            self.surf.blit(self.project_files_view.surf, (0, win_size[1] * (1 - dim.bottom)))
            self.surf.blit(self.viewport.surf, (win_size[0] * dim.left, win_size[1] * dim.top))

            # Write the UI surface to the texture and write viewport dimensions
            self.texture.write(self.surf.get_view('1'))
            self.program['viewportRect'].write(glm.vec4(dim.left, dim.bottom, dim.width, dim.height))
            self.do_update_texture = False

        # Render the UI texture
        self.ctx.screen.use()
        self.vao.render()

    def render_viewports(self):
        """
        Renders the individual viewport/window surfaces
        """
        
        self.viewport.render()
        self.hierarchy.render()
        self.inspector.render()
        self.toolbar.render()
        self.project_files_view.render()

    def set_viewports(self):
        """
        Sets the surfaces of each viewport/window
        """
        
        self.viewport.set_surf()
        self.hierarchy.set_surf()
        self.inspector.set_surf()
        self.toolbar.set_surf()
        self.project_files_view.set_surf()

        self.program['viewportRect'].write(glm.vec4(self.editor.viewport_dim.left, self.editor.viewport_dim.top, self.editor.viewport_dim.width, self.editor.viewport_dim.height))

        self.refresh()

    def set_surface_texture(self):
        """
        Makes a moderngl texture from the UI surface. Binds it and the scene framebuffer
        """
        
        self.surf = pg.Surface(self.editor.engine.win_size).convert_alpha()

        if self.texture: self.texture.release()

        self.texture = self.ctx.texture(self.surf.get_size(), 4)
        self.texture.filter = (mgl.NEAREST, mgl.NEAREST)
        self.texture.swizzel = 'RGBA'

        self.program['screenTexture'] = 0
        self.texture.use(location=0)
        self.program['engineTexture'] = 1
        self.editor.engine.project.current_scene.vao_handler.frame_texture.use(location=1)

        self.refresh()

    def refresh(self):
        """
        Sets the update texutre bit so that the screen texture is rendered
        """
        
        self.do_update_texture = True

    def window_resize(self):
        # Update views and texture
        self.set_viewports()
        self.set_surface_texture()

        # Update assets
        self.logo = pg.transform.scale(self.logo_source, (self.editor.viewport_dim.top * self.editor.engine.win_size[1] * .8, self.editor.viewport_dim.top * self.editor.engine.win_size[1] * .8))