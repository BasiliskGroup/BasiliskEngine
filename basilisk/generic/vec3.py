import glm
import numpy as np

class Vec3():
    def __init__(self, *args, callback=None):
        self.callback = callback
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
            if isinstance(args[0], Vec3):
                self.data = glm.vec3(args[0].data)
                self.callback = args[0].callback
            elif isinstance(args[0], glm.vec3): self.data = glm.vec3(args[0])
            elif isinstance(args[0], tuple) or isinstance(args[0], list) or isinstance(args[0], np.ndarray): 
                if len(args[0]) != 3: raise ValueError(f'Vec3: Expected 3 values from incoming vector, got {len(args[0])}')
                self.data = glm.vec3(args[0])
            else: raise ValueError(f'Vec3: Unexpected incoming vector type {args[0]}')
        elif len(args) == 3: self.data = glm.vec3(args[0], args[1], args[2])
        else: raise ValueError(f'Vec3: Expected either 1 vector or 3 numbers, got {len(args)} values')

    # override _= operators
    def __iadd__(self, other):
        if isinstance(other, glm.vec3): self.data += other
        elif isinstance(other, tuple) or isinstance(other, list) or isinstance(other, np.ndarray):
            if len(other) != 3: raise ValueError(f'Vec3: Number of added values must be 3, got {len(other)}')
            self.data += other
        elif isinstance(other, Vec3): self.data += other.data
        else: raise ValueError(f'Vec3: Not an accepted type for addition, got {type(other)}')
        return self
    
    def __isub__(self, other):
        if isinstance(other, glm.vec3): self.data -= other
        elif isinstance(other, tuple) or isinstance(other, list) or isinstance(other, np.ndarray):
            if len(other) != 3: raise ValueError(f'Vec3: Number of added values must be 3, got {len(other)}')
            self.data -= other
        elif isinstance(other, Vec3): self.data -= other.data
        else: raise ValueError(f'Vec3: Not an accepted type for addition, got {type(other)}')
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
        if int(index) != index: raise IndexError(f'Vec3: index must be an int, got {type(index)}') # check if index is a float
        if index < 0 or index > 2: raise IndexError(f'Vec3: index out of bounds, got {index}')
        return self.data[index]
    
    def __setitem__(self, index, value): 
        if int(index) != index: raise IndexError(f'Vec3: index must be an int, got {type(index)}') # check if index is a float
        if index < 0 or index > 2: raise IndexError(f'Vec3: index out of bounds, got {index}')
        try: self.data[index] = value
        except: raise ValueError(f'Vec3: Invalid element type, got {type(value)}')
        
    def __repr__(self):
        return str(self.data)

    def __iter__(self):
        return iter((self.x, self.y, self.z))
    
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
        self._data = value
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