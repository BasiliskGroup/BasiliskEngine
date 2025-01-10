import glm
import numpy as np


class Quat():
    def __init__(self, *args, callback=None):
        self.callback = callback
        self.prev_data = glm.quat(1, 0, 0, 0)
        self.set_data(*args)
       
    def normalize(self):
        """
        Inplace normalizes the vector
        """
        self.data = glm.normalize(self.data)
        
    def set_data(self, *args):
        """
        Sets the internal vector inplace
        """
        # overload constructor TODO nvernest this, definitely possible
        if len(args) == 1:
            if isinstance(args[0], Quat):
                self.data = glm.quat(args[0].data)
                self.callback = args[0].callback
            elif isinstance(args[0], glm.quat): self.data = glm.quat(args[0])
            elif isinstance(args[0], tuple) or isinstance(args[0], list) or isinstance(args[0], np.ndarray): 
                if len(args[0]) != 4 and len(args[0]) != 3: raise ValueError(f'Quat: Expected 3 or 4 values from incoming vector, got {len(args[0])}')
                self.data = glm.quat(args[0])
            else: raise ValueError(f'Quat: Unexpected incoming vector type {args[0]}')
        elif len(args) == 4: self.data = glm.quat(*args)
        else: raise ValueError(f'Quat: Expected either 1 vector or 4 numbers, got {len(args)} values')

    # override _= operators
    def __iadd__(self, other):
        if isinstance(other, glm.quat): self.data += other
        elif isinstance(other, tuple) or isinstance(other, list) or isinstance(other, np.ndarray):
            if len(other) != 4 and len(other) != 3: raise ValueError(f'Quat: Number of added values must be 3 or 4, got {len(other)}')
            self.data += other
        elif isinstance(other, Quat): self.data += other.data
        else: raise ValueError(f'Quat: Not an accepted type for addition, got {type(other)}')
        return self
    
    def __isub__(self, other):
        if isinstance(other, glm.quat): self.data -= other
        elif isinstance(other, tuple) or isinstance(other, list) or isinstance(other, np.ndarray):
            if len(other) != 4 and len(other) != 3: raise ValueError(f'Quat: Number of added values must be 3 or 4, got {len(other)}')
            self.data -= other
        elif isinstance(other, Quat): self.data -= other.data
        else: raise ValueError(f'Quat: Not an accepted type for addition, got {type(other)}')
        return self
    
    def __imul__(self, other):
        # TODO add checks for number types
        self.data *= other
        return self
    
    def __idiv__(self, other): 
        # TODO add checks for number types
        self.data /= other
        return self
    
    def __ifloordiv__(self, other): 
        # TODO add checks for number types
        self.data //= other
        return self
    
    # override [_] accessor
    def __getitem__(self, index): 
        if int(index) != index: raise IndexError(f'Quat: index must be an int, got {type(index)}') # check if index is a float
        if index < 0 or index > 3: raise IndexError(f'Quat: index out of bounds, got {index}')
        return self.data[index]
    
    def __setitem__(self, index, value): 
        if int(index) != index: raise IndexError(f'Quat: index must be an int, got {type(index)}') # check if index is a float
        if index < 0 or index > 3: raise IndexError(f'Quat: index out of bounds, got {index}')
        try: self.data[index] = value
        except: raise ValueError(f'Quat: Invalid element type, got {type(value)}')
        
    def __repr__(self):
        return str(self.data)

    def __iter__(self):
        return iter((self.x, self.y, self.z))
    
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
    def data(self, value: glm.vec3):
        self._data = value
        
        cur = self._data
        prev = self.prev_data
        thresh = 1e-6
        
        if self.callback and (abs(cur.w - prev.w) > thresh or abs(cur.x - prev.x) > thresh or abs(cur.y - prev.y) > thresh or abs(cur.z - prev.z) > thresh): 
            self.prev_data = self._data            
            self.callback()
        
    @w.setter
    def w(self, value):
        self.data.w = value
        if self.callback: self.callback()

    @x.setter
    def x(self, value):
        self.data.x = value
        if self.callback: self.callback()
        
    @y.setter
    def y(self, value):
        self.data.y = value
        if self.callback: self.callback()
        
    @z.setter
    def z(self, value):
        self.data.z = value
        if self.callback: self.callback()