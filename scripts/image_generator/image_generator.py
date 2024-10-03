import os
import numpy as np
import moderngl as mgl
import glm
from PIL import Image
from pyobjloader import load_model
import pygame as pg

# Camera view constants
FOV = 50  # Degrees
NEAR = 0.1
FAR = 350

class ImageGenerator():
    def __init__(self, ctx) -> None:
        """
        Initalize all context attributes
        """

        # Create a headless context
        self.ctx = ctx
        self.ctx.enable(mgl.DEPTH_TEST | mgl.CULL_FACE)  # Render only what is visible

        # Set all the ModernGL attrbiutes
        self.set_program()
        self.set_vao()
        self.set_framebuffer()
        self.refresh()

    def refresh(self):
        """
        Sets the matricies, uniforms, and gets the bounding box
        """
        
        self.set_matricies()
        self.set_uniforms()

    def set_program(self):
        """
        Creates an MGL program with the default shader.
        """
        
        # Read shaders from files
        vert = open('scripts/image_generator/shaders/default.vert').read()
        frag = open('scripts/image_generator/shaders/default.frag').read()
        
        # Create a program
        self.program = self.ctx.program(vertex_shader=vert, fragment_shader=frag)

    def set_vao(self, model='cow', vbo=None):
        """
        Loads the model, creates a vertex buffer, then creates a VAO from VBO and current program
        """

        if model:
            self.model = load_model(f'models/{model}.obj')
        else:
            self.model = vbo

        if len(self.model.vertex_data[0] == 8):
            data = self.model.vertex_data
            data[:,3:6] = data[:,5:8]
            data = np.array(data[:,:6], dtype='f4')
        else:
            data = self.model.vertex_data

        self.vbo = self.ctx.buffer(data)
        
        self.vao = self.ctx.vertex_array(self.program, [(self.vbo, '3f 3f', *['in_position', 'in_normal'])])

    def set_matricies(self):
        """
        Sets the matricies for the camera MVP
        """
        
        # Matrix continaing object pos/rot/scale data (currently is just identity though)
        scale_x = [self.model.vertex_data.max(axis=0)[0], self.model.vertex_data.min(axis=0)[0]]
        scale_y = [self.model.vertex_data.max(axis=0)[1], self.model.vertex_data.min(axis=0)[1]]
        scale_z = [self.model.vertex_data.max(axis=0)[2], self.model.vertex_data.min(axis=0)[2]]

        scale = max(scale_x[0], abs(scale_x[1]), scale_y[0], abs(scale_y[1]), scale_z[0], abs(scale_z[1]))
        scale = max(scale_x[0] - scale_x[1], scale_y[0] - scale_y[1], scale_z[0] -scale_z[1])

        self.m_model = glm.mat4()
        self.m_model = glm.scale(self.m_model, glm.vec3(1 / scale))
        self.m_model = glm.translate(self.m_model, -glm.vec3((scale_x[1] + scale_x[0])/2, (scale_y[1] + scale_y[0])/2, (scale_z[1] + scale_z[0])/2))
        
        # The projection matrix
        self.m_proj = glm.perspective(glm.radians(FOV), 1, NEAR, FAR)
        
        # Camera view matrix, which is set at a random position and pointed at the object
        dist = 1.2
        self.m_view = glm.lookAt(glm.vec3(dist, dist/1.25, dist), glm.vec3(0, 0, 0), glm.vec3(0, 1, 0))

    def set_uniforms(self):
        """
        Writes the MVP to the program
        """
        
        self.program['m_model'].write(self.m_model)
        self.program['m_view'].write(self.m_view)
        self.program['m_proj'].write(self.m_proj)

    def set_framebuffer(self):
        """
        Creates framebuffer to be used as a render target and to copy to image 
        """
        
        frame_texture = self.ctx.texture((512, 512), components=4)
        depth_texture = self.ctx.depth_texture((512, 512))
        self.framebuffer   = self.ctx.framebuffer([frame_texture], depth_texture)
    
    def render(self):
        """
        Renders the object
        """
        
        self.framebuffer.use()
        self.ctx.clear(.1, .1, .1)
        self.vao.render()

    def generate_folder(self, folder: str) -> list:
        """
        Generates images of all the .obj files in the given folder. 
        Returns a list of images
        """

        images = []

        for file in os.listdir(folder):
            filename = os.fsdecode(file)
            if not filename.endswith(".obj"): continue

            self.set_vao(file[:-4])
            self.refresh()
            self.render()

            data = self.framebuffer.read(components=3, alignment=1)
            img = Image.frombytes('RGB', self.framebuffer.size, data).transpose(Image.FLIP_TOP_BOTTOM)
            img.save(f'scripts/image_generator/test_save/{file[:-4]}.png')  # Save image to disk            images.append(img)

        return images

    def generate_file(self, file: str) -> list:
        """
        Generates a single .obj file.  
        Returns an image of the model
        """

        images = []

        filename = os.fsdecode(file)
        if not filename.endswith(".obj"): return -1

        self.set_vao(file[:-4])
        self.refresh()
        self.render()

        data = self.framebuffer.read(components=3, alignment=1)
        img = pg.image.frombytes(data, self.framebuffer.size, 'RGB')
        return img
    
    def generate_vbos(self, vbos: list) -> list:
        """
        Generates images of all the .obj files in the given folder. 
        Returns a list of images
        """

        images = []

        for vbo in vbos:
            self.set_vao(None, vbo=vbo)
            self.refresh()
            self.render()

            data = self.framebuffer.read(components=3, alignment=1)
            img = pg.image.frombytes(data, self.framebuffer.size, 'RGB').convert_alpha()
            img = pg.transform.flip(img, flip_x=False, flip_y=True)
            images.append(img)

        return images

if __name__ == '__main__':
    image_generator = ImageGenerator()
    image_generator.generate_folder('objects')