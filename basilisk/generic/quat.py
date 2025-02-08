import glm
import numpy as np
from .abstract_custom import Custom


class Quat(Custom):
    def __init__(self, *args, callback=None):
        self.callback = callback
        self.prev_data = glm.quat(1, 0, 0, 0)
        
        if len(args) == 1:
            
            if isinstance(args[0], Quat):
                self.data = glm.quat(args[0].data)
                self.prev_data = glm.quat(args[0].prev_data)
                self.callback = args[0].callback
                
            elif isinstance(args[0], glm.quat): 
                self.data = args[0]
                
            elif isinstance(args[0], tuple) or isinstance(args[0], list) or isinstance(args[0], np.ndarray): 
                assert 2 < len(args[0]) < 5, f'Quat: Expected 3 or 4 values from incoming vector, got {len(args[0])}'
                self.data = glm.quat(args[0])
                
            else: 
                try: self.data = glm.quat(args[0])
                except: raise ValueError(f'Quat: Unexpected incoming quaternion type {args[0]}')
            
        elif 2 < len(args) < 5: self.data = glm.quat(*args)
        else: raise ValueError(f'Quat: Expected either a vector, quaternion, or 3 or 4 numbers, got {len(args)} values')
        
    def apply_function(self, other, func, func_name):
        quat = glm.quat(self.data)
        
        if isinstance(other, (glm.vec3, glm.quat)):
            quat = func(quat, other)
            
        elif isinstance(other, (tuple, list, np.ndarray)):
            assert 2 < len(other) < 5, f'Quat: Expected 3 or 4 values from incoming vector, got {len(other)}'
            quat = func(quat, other)
            
        elif isinstance(other, Custom):
            quat = func(quat, other.data)
            
        else: 
            try: quat = func(quat, other)
            except: raise ValueError(f'Quat: Not an accepted type for {func_name}, got {type(other)}')
        return Quat(quat)

    # unary operators
    def __neg__(self):
        return Quat(-self.data, callback=self.callback)
    
    def __abs__(self):
        return Quat(abs(self.data), callback=self.callback)
    
    # accessor functions
    def __getitem__(self, index): 
        assert int(index) == index, f'Quat: index must be an int, got {type(index)}' # check if index is a float
        assert 0 <= index <= 3, f'Quat: index out of bounds, got {index}'
        return self.data[index]
    
    def __setitem__(self, index, value): 
        assert int(index) == index, f'Quat: index must be an int, got {type(index)}' # check if index is a float
        assert 0 <= index <= 3, f'Quat: index out of bounds, got {index}'
        try: self.data[index] = value
        except: raise ValueError(f'Quat: Invalid element type, got {type(value)}')
        
    def __delitem__(self, index): # cannot delete an index from a quaternion, so we set the value to zero instead
        assert int(index) == index, f'Quat: index must be an int, got {type(index)}' # check if index is a float
        assert 0 <= index <= 3, f'Quat: index out of bounds, got {index}'
        self.data[index] = 0
        
    def __len__(self):
        return 4
    
    def __iter__(self):
        return iter(self.data)
    
    def __contains__(self, item):
        return item in self.data

    # override str operators
    def __repr__(self):
        return 'bsk' + str(self.data)
    
    def __str__(self):
        return 'bsk ' + str(self.data)
    
    @property
    def data(self): return self._data
    @property
    def w(self): return self.data.w
    @property
    def x(self): return self.data.x
    @property
    def y(self): return self.data.y
    @property
    def z(self): return self.data.z
    
    @data.setter
    def data(self, value: glm.vec3 | glm.quat):
        self._data = glm.quat(value)
        cur = self._data
        prev = self.prev_data
        thresh = 1e-6
        
        if self.callback and (abs(cur.w - prev.w) > thresh or abs(cur.x - prev.x) > thresh or abs(cur.y - prev.y) > thresh or abs(cur.z - prev.z) > thresh): 
            self.prev_data = glm.quat(self._data)            
            self.callback()
        self.normalize()
        
    @w.setter
    def w(self, value):
        self.data.w = value
        if self.callback and abs(value - self.prev_data.w) > 1e-6: 
            self.prev_data.w = value
            self.callback()
        self.normalize()

    @x.setter
    def x(self, value):
        self._data.x = value
        if self.callback and abs(value - self.prev_data.x) > 1e-6: 
            self.prev_data.x = value
            self.callback()
        self.normalize()
        
    @y.setter
    def y(self, value):
        self._data.y = value
        if self.callback and abs(value - self.prev_data.y) > 1e-6: 
            self.prev_data.y = value
            self.callback()
        self.normalize()
        
    @z.setter
    def z(self, value):
        self._data.z = value
        if self.callback and abs(value - self.prev_data.z) > 1e-6: 
            self.prev_data.z = value
            self.callback()
        self.normalize()