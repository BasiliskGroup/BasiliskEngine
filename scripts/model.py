from scripts.generic.data_types import vec3


CHUNK_SIZE = 40


class Model:
    def __init__(self, handler, vbo, material='base', position: tuple=(0, 0, 0), rotation: tuple=(0, 0, 0), scale: tuple=(1, 1, 1)) -> None:
        # Rendering specifications
        self.__handler = handler
        self.vbo     = vbo

        # Chunk that the model is in
        self.chunk = (position[0] // CHUNK_SIZE, position[1] // CHUNK_SIZE, position[2] // CHUNK_SIZE)
        self.prev_chunk = self.chunk
        
        self.material = material
        
        # Variables for detecting attribute changes
        self.__prev_position = (-100, -100, -100)
        self.__prev_rotation = (-100, -100, -100)
        self.__prev_scale    = (-100, -100, -100)

        # Model matrix vectors
        self.position = vec3(position, self.update_position)
        self.rotation = vec3(rotation, self.update_rotation)
        self.scale    = vec3(scale   , self.update_scale)  
        
        self.base_volume = 8

    @property
    def position(self): return self._position
    @property
    def scale(self): return self._scale
    @property
    def rotation(self): return self._rotation
    @property
    def material(self): return self._material
    @property
    def x(self): return self.position.x
    @property
    def y(self): return self.position.y
    @property
    def z(self): return self.position.z

    @position.setter
    def position(self, value):
        self._position = value
        self.update_position()
    @scale.setter
    def scale(self, value):
        self._scale = value
        self.update_scale()
    @rotation.setter
    def rotation(self, value):
        self._rotation = value
        self.update_rotation()
    @material.setter
    def material(self, value):
        self._material = self.__handler.scene.material_handler.material_ids[value]
        self.__handler.updated_chunks.add(self.chunk)
    @x.setter
    def x(self, value): self.position.x = value
    @y.setter
    def y(self, value): self.position.y = value
    @z.setter
    def z(self, value): self.position.z = value
    

    def update_position(self):
        """
        Checks if the object has moved enough to update the chunk mesh
        """
                    
        self.chunk = (self.x // CHUNK_SIZE, self.y // CHUNK_SIZE, self.z // CHUNK_SIZE)

        if abs(self.__prev_position[0] - self.position[0]) < 0.001 and abs(self.__prev_position[1] - self.position[1]) < 0.001 and abs(self.__prev_position[2] - self.position[2]) < 0.001: return False   

        if self.prev_chunk != self.chunk:
            if self.chunk not in self.__handler.chunks:
                self.__handler.chunks[self.chunk] = []
            
            self.__handler.chunks[self.chunk].append(self)
            self.__handler.chunks[self.prev_chunk].remove(self)

        self.__handler.updated_chunks.add(self.prev_chunk)
        self.__handler.updated_chunks.add(self.chunk)

        self.prev_chunk = self.chunk
        self.__prev_position = tuple(self.position[:])

    def update_scale(self):
        """
        Checks if the object has been scaled enough to update the chunk mesh
        """

        if abs(self.__prev_scale[0] - self.scale.x) < 0.001 and abs(self.__prev_scale[1] - self.scale.y) < 0.001 and abs(self.__prev_scale[2] - self.scale.z) < 0.001: return False  
        
        self.__handler.updated_chunks.add(self.chunk)

        self.__prev_scale = tuple(self.scale[:])

    def update_rotation(self):
        """
        Checks if the object has been rotated enough to update the chunk mesh
        """

        if abs(self.__prev_rotation[0] - self.rotation.x) < 0.001 and abs(self.__prev_rotation[1] - self.rotation.y) < 0.001 and abs(self.__prev_rotation[2] - self.rotation.z) < 0.001: return False  
        
        self.__handler.updated_chunks.add(self.chunk)

        self.__prev_rotation = tuple(self.rotation[:])

    def __repr__(self) -> str:
        return f'<Object: {self.position[0]},{self.position[1]},{self.position[2]}>'
    
    def get_volume(self) -> float:
        return self.base_volume * self.scale.x * self.scale.y * self.scale.z