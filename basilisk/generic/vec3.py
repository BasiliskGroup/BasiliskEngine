import glm
import numpy as np
from .abstract_custom import Custom

class Vec3(Custom):
    def __init__(self, *args, callback=None):
        self.callback = callback
        self.prev_data = glm.vec3(0.0)
        
        if len(args) == 1:
            
            if isinstance(args[0], Vec3):
                self.data = glm.vec3(args[0].data)
                self.prev_data = glm.vec3(args[0].prev_data)
                self.callback = args[0].callback
                
            elif isinstance(args[0], glm.vec3): 
                self.data = args[0]
                
            elif isinstance(args[0], tuple) or isinstance(args[0], list) or isinstance(args[0], np.ndarray): 
                assert len(args[0]) == 3, f'Vec3: Expected 3 values from incoming vector, got {len(args[0])}'
                self.data = glm.vec3(args[0])
                
            else: 
                try: self.data = glm.vec3(args[0])
                except: raise ValueError(f'Vec3: Unexpected incoming vector type {args[0]}')
            
        elif len(args) == 3: self.data = glm.vec3(*args)
        else: raise ValueError(f'Vec3: Expected either 1 vector or 3 numbers, got {len(args)} values')
       
    def apply_function(self, other, func, func_name):
        vec = glm.vec3(self.data)
        
        if isinstance(other, (glm.vec3, glm.quat)): 
            vec = func(vec, other) 
            
        elif isinstance(other, tuple) or isinstance(other, list) or isinstance(other, np.ndarray):
            assert len(other) == 3, f'Vec3: Number of added values must be 3, got {len(other)}'
            vec = func(vec, other) 
            
        elif isinstance(other, Custom): # perserve self.callback over other.callback. this should never be done by the user
            vec = func(vec, other.data) 
            
        else: 
            try: vec = func(vec, other) 
            except: raise ValueError(f'Vec3: Not an accepted type for {func_name}, got {type(other)}')
        return Vec3(vec)
        
    # unary operators
    def __neg__(self):
        return Vec3(-self.data, callback=self.callback)
    
    def __abs__(self):
        return Vec3(abs(self.data), callback=self.callback)
    
    # accessor functions
    def __getitem__(self, index): 
        assert int(index) == index, f'Vec3: index must be an int, got {type(index)}' # check if index is a float
        assert 0 <= index <= 2, f'Vec3: index out of bounds, got {index}'
        return self.data[index]
    
    def __setitem__(self, index, value): 
        assert int(index) == index, f'Vec3: index must be an int, got {type(index)}' # check if index is a float
        assert 0 <= index <= 2, f'Vec3: index out of bounds, got {index}'
        try: self.data[index] = value
        except: raise ValueError(f'Vec3: Invalid element type, got {type(value)}')
        
    def __delitem__(self, index): # index in a vec cannot be deleted, so we default to zero
        assert int(index) == index, f'Vec3: index must be an int, got {type(index)}' # check if index is a float
        assert 0 <= index <= 2, f'Vec3: index out of bounds, got {index}'
        self.data[index] = 0
        
    def __len__(self):
        return 3
        
    def __iter__(self):
        return iter(self.data)
    
    def __contains__(self, item):
        return item in self.data
    
    # override str operators
    def __repr__(self):
        return 'bsk ' + str(self.data)
    
    def __str__(self):
        return 'bsk ' + str(self.data)
    
    @property
    def data(self): return self._data
    @property
    def x(self): return self.data.x
    @property
    def y(self): return self.data.y
    @property
    def z(self): return self.data.z
    
    @data.setter
    def data(self, value: glm.vec3):
        self._data = glm.vec3(value)
        cur = self._data
        prev = self.prev_data
        thresh = 1e-6
        
        if self.callback and (abs(cur.x - prev.x) > thresh or abs(cur.y - prev.y) > thresh or abs(cur.z - prev.z) > thresh): 
            self.prev_data = glm.vec3(self._data)
            self.callback()

    @x.setter
    def x(self, value):
        self._data.x = value
        if self.callback and abs(value - self.prev_data.x) > 1e-6: 
            self.prev_data.x = value
            self.callback()
        
    @y.setter
    def y(self, value):
        self._data.y = value
        if self.callback and abs(value - self.prev_data.y) > 1e-6: 
            self.prev_data.y = value
            self.callback()
        
    @z.setter
    def z(self, value):
        self._data.z = value
        if self.callback and abs(value - self.prev_data.z) > 1e-6: 
            self.prev_data.z = value
            self.callback()
        
class Node():
    
    def __init__(self, pos):
        
        def callback(): print('calling back')
        self.pos_callback = callback
        self._pos = Vec3(pos, callback=self.pos_callback)
        
    @property
    def pos(self): return self._pos
    
    @pos.setter
    def pos(self, value): 
        if isinstance(value, Vec3): self._pos.data = value.data
        else: self._pos.data = value