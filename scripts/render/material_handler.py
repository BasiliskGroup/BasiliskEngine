import glm
import numpy as np

class MaterialHandler:
    def __init__(self, scene) -> None:
        self.scene          = scene
        self.texture_ids    = scene.project.texture_handler.texture_ids
        self.programs       = scene.vao_handler.shader_handler.programs
        self.materials      = {}
        self.material_ids   = {}

        #self.add('cow', texture="cow")
        #self.add('brick', texture='brick', normal_map='brick_normal')

    def add(self, name="base", color: tuple=(1, 1, 1), specular: float=1, specular_exponent: float=32, alpha: float=1, texture=None, normal_map=None):
        mtl = Material(self, color, specular, specular_exponent, alpha, texture, normal_map)
        self.material_ids[name] = len(self.materials)
        self.materials[name] = mtl

    def get(self, value):
        if type(value) == int:
            return self.materials[list(self.material_ids.keys())[value]]

    def write(self, program):
        program = self.programs[program]
        self.make_texture(program)
        for mtl_name in list(self.materials.keys()):
            mtl = self.materials[mtl_name]
            mtl.write(program, self.texture_ids, self.material_ids[mtl_name])

    def make_texture(self, program):
        if not len(self.materials): return

        # (3i, 1i, 1i, 1i, 3i, 3i)
        texture_data = np.zeros(shape=(len(self.materials), 12), dtype="f")

        texture_ids = self.scene.project.texture_handler.texture_ids

        for i, mtl in enumerate(list(self.materials.values())):
            texture_data[i][:3]  = mtl.color.x, mtl.color.y, mtl.color.z
            texture_data[i][3]   = mtl.specular.value
            texture_data[i][4]   = mtl.specular_exponent.value
            texture_data[i][5]   = mtl.alpha.value
            texture_data[i][6]   = mtl.has_texture.value
            texture_data[i][7:9] = texture_ids[mtl.texture] if mtl.texture else [0, 0]
            texture_data[i][9]   = mtl.has_normal_map.value
            texture_data[i][10:] = texture_ids[mtl.normal_map] if mtl.normal_map else [0, 0]

        texture_buffer = self.scene.ctx.buffer(texture_data)

        self.mtl_texture = self.scene.ctx.texture((texture_buffer.size, 1), components=1, data=texture_buffer.read())

        program[f'materialsTexture'] = 9
        self.mtl_texture.use(location=9)

class Material:
    def __init__(self, handler, color: tuple, specular:float, specular_exponent: float, alpha: float, texture=None, normal_map=None) -> None:
        self.handler = handler
        # Numberic attributes
        self.color:             glm.vec3    = color
        self.specular:          glm.float32 = specular
        self.specular_exponent: glm.float32 = specular_exponent
        self.alpha:             glm.float32 = alpha

        # Texture Maps
        if not texture:
            self.has_texture = False
            self.texture     = None
        else:
            self.has_texture  = True
            self.texture: str = texture

        if not normal_map:
            self.has_normal_map = False
            self.normal_map     = None
        else:
            self.has_normal_map  = True
            self.normal_map: str = normal_map

    def write(self, program, texture_ids, i=0):
        program[f'materials[{i}].color'           ].write(self.color)
        program[f'materials[{i}].specular'        ].write(self.specular)
        program[f'materials[{i}].specularExponent'].write(self.specular_exponent)
        program[f'materials[{i}].alpha'           ].write(self.alpha)

        program[f'materials[{i}].hasAlbedoMap'  ].write(self.has_texture)
        program[f'materials[{i}].hasNormalMap'  ].write(self.has_normal_map)

        if self.has_texture  : program[f'materials[{i}].albedoMap'  ].write(glm.vec2(texture_ids[self.texture]))
        if self.has_normal_map  : program[f'materials[{i}].normalMap'  ].write(glm.vec2(texture_ids[self.normal_map]))

    @property
    def color(self): return self._color
    @property
    def r(self): return self._color.x
    @property
    def g(self): return self._color.y
    @property
    def b(self): return self._color.z
    @property
    def specular(self): return self._specular
    @property
    def specular_exponent(self): return self._specular_exponent
    @property
    def alpha(self): return self._alpha
    @property
    def has_texture(self): return self._has_texture
    @property
    def has_normal_map(self): return self._has_normal_map
    @property
    def texture(self): return self._texture
    @property
    def normal_map(self): return self._normal_map

    @color.setter
    def color(self, value):
        self._color = glm.vec3(value)
        self.handler.write('batch')
    @r.setter
    def r(self, value):
        self._color.x = value
        self.handler.write('batch')
    @g.setter
    def g(self, value):
        self._color.y = value
        self.handler.write('batch')
    @b.setter
    def b(self, value):
        self._color.z = value
        self.handler.write('batch')
    @specular.setter
    def specular(self, value):
        self._specular = glm.float32(value)
        self.handler.write('batch')
    @specular_exponent.setter
    def specular_exponent(self, value):
        self._specular_exponent = glm.float32(value)
        self.handler.write('batch')
    @alpha.setter
    def alpha(self, value):
        self._alpha = glm.float32(value)
    @has_texture.setter
    def has_texture(self, value):
        self._has_texture = glm.int32(int(value))
    @has_normal_map.setter
    def has_normal_map(self, value):
        self._has_normal_map = glm.int32(int(value))
    @texture.setter
    def texture(self, value):
        if value: self.has_texture = True
        else: self.has_texture = False
        self._texture = value
    @normal_map.setter
    def normal_map(self, value):
        if value: self.has_normal_map = True
        else: self.has_normal_map = False
        self._normal_map = value