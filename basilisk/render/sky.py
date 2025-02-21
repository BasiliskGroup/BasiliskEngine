import numpy as np
from PIL import Image as PIL_Image
from .shader import Shader

class Sky:
    texture_cube=None
    vbo          = None
    vao          = None
    def __init__(self, engine, sky_texture: str | list=None):
        """
        Handler for all skybox rendering
        """
        
        self.engine = engine
        self.ctx   = engine.ctx
        
        if not sky_texture: sky_texture = engine.root + '/bsk_assets/skybox.png'

        self.set_renderer()
        self.set_texture(sky_texture)

    def render(self):
        """
        Render the skybox to current render destination
        """
        self.vao.render()

    def write(self):
        # Write the texture cube to the sky shader
        self.shader.program['skyboxTexture']  = 8
        self.texture_cube.use(location = 8)

        shader = self.engine.shader
        if 'skyboxTexture' not in shader.uniforms: return
        shader.program['skyboxTexture'] = 8
        self.texture_cube.use(location = 8)


    def set_texture(self, skybox_images: list):
        """
        Sets the skybox texture. Can either be set with 6 images for each skybox side or a single skybox image.
        List items should be string paths.
        The six images should be should be in the following order: right, left, top, bottom, front, back
        """

        # Release any existing memory
        if self.texture_cube: self.texture_cube.release()

        # Function-Scoped data
        images = None

        # Given a sinlge image for the skybox        
        if isinstance(skybox_images, str) or ((isinstance(skybox_images, list) or isinstance(skybox_images, tuple)) and len(skybox_images) == 1):
            path = skybox_images if isinstance(skybox_images, str) else skybox_images[0]
            
            # Verify the path type
            if not isinstance(path, str): raise ValueError(f"Skybox: Invalid image path type {type(path)}")

            image = PIL_Image.open(path).convert('RGB')
            width, height = image.size[0] // 4, image.size[1] // 3

            images = [image.crop((x * width, y * height, (x + 1) * width, (y + 1) * height)) for x, y in [(2, 1), (0, 1), (1, 0), (1, 2), (1, 1), (3, 1)]]

        # Given a list of images for the skybox
        elif isinstance(skybox_images, list) or isinstance(skybox_images, tuple):
            # Verify the correct number of images was given
            if len(skybox_images) != 6: raise ValueError("Skybox: Invalid number of images for skybox. Expected 1 or 6")
            # Verify the all image path types            
            if not all([isinstance(path, str) for path in skybox_images]): raise ValueError(f"Skybox: Invalid image path type {type(path)}")

            images = [PIL_Image.open(path).convert('RGB') for path in skybox_images]

        else:
            raise ValueError(f"Skybox: Invalid skybox type {type(skybox_images)}. Expected list of string paths or a single image")
        
        # Create a texture map from the images
        size = min(images[0].size)
        size = (size, size)
        images = [img.resize(size) for img in images]
        images = [img.tobytes() for img in images]
        self.texture_cube = self.ctx.texture_cube(size=size, components=3, data=None)
        for i, data in enumerate(images):
            self.texture_cube.write(face=i, data=data)

    def set_renderer(self):
        """
        
        """
        
        # Release any existing memory
        if self.vbo: self.vbo.release()
        if self.vao: self.vao.release()
        
        # Get the cube vertex data
        vertices = [(-1, -1, 1), ( 1, -1,  1), (1,  1,  1), (-1, 1,  1),
                    (-1, 1, -1), (-1, -1, -1), (1, -1, -1), ( 1, 1, -1)]

        indices = [(0, 2, 3), (0, 1, 2),
                   (1, 7, 2), (1, 6, 7),
                   (6, 5, 4), (4, 7, 6),
                   (3, 4, 5), (3, 5, 0),
                   (3, 7, 4), (3, 2, 7),
                   (0, 6, 1), (0, 5, 6)]
        
        vertex_data = np.array([vertices[ind] for trigangle in indices for ind in trigangle], dtype='f4')
        vertex_data = np.flip(vertex_data, 1).copy(order='C')

        # Create a renderable vao
        self.vbo     = self.ctx.buffer(vertex_data)
        root = self.engine.root
        self.shader = self.engine.shader_handler.add(Shader(self.engine, root + '/shaders/sky.vert', root + '/shaders/sky.frag'))
        self.vao     = self.ctx.vertex_array(self.shader.program, [(self.vbo, '3f', 'in_position')], skip_errors=True)

    def __del__(self):
        """
        Releases all data used by the skybox
        """
        
        if self.texture_cube: self.texture_cube.release()
        if self.vbo: self.vbo.release()
        if self.vao: self.vao.release()