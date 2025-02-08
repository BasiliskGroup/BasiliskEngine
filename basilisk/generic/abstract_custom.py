import glm
import numpy as np


class Custom():
    
    def normalize(self):
        """
        Inplace normalizes the vector
        """
        self._data = glm.normalize(self._data)
        return self
    
    def apply_function(): ... # will be overridden by child custom classes
    
    # math functions
    def add(self, other):
        def func(a, b): return a + b
        return self.apply_function(other, func, 'addition')
    
    def subtract(self, other):
        def func(a, b): return a - b
        return self.apply_function(other, func, 'subtraction')
    
    def rhs_subtract(self, other):
        def func(a, b): return b - a
        return self.apply_function(other, func, 'subtraction')
    
    def multiply(self, other):
        def func(a, b): return a * b
        return self.apply_function(other, func, 'multiplication')
    
    def divide(self, other):
        def func(a, b): return a / b
        return self.apply_function(other, func, 'division')
    
    def rhs_divide(self, other):
        def func(a, b): return b / a
        return self.apply_function(other, func, 'division')
    
    def floor_divide(self, other):
        def func(a, b): return a // b
        return self.apply_function(other, func, 'division')
    
    def rhs_floor_divide(self, other):
        def func(a, b): return b // a
        return self.apply_function(other, func, 'division')
    
    def mod(self, other):
        def func(a, b): return a % b
        return self.apply_function(other, func, 'modulus')
    
    def rhs_mod(self, other):
        def func(a, b): return b % a
        return self.apply_function(other, func, 'modulus')
    
    def pow(self, other):
        def func(a, b): return a ** b
        return self.apply_function(other, func, 'power')
    
    def rhs_pow(self, other):
        def func(a, b): return b ** a
        return self.apply_function(other, func, 'power')

    def __add__(self, other):  return self.add(other) # this + that
    def __radd__(self, other): return self.add(other) # that + this
    def __iadd__(self, other): # this += that
        self = self.add(other) 
        return self
    
    def __sub__(self, other):  return self.subtract(other)
    def __rsub__(self, other): return self.rhs_subtract(other) # non-commutative
    def __isub__(self, other): 
        self = self.subtract(other)
        return self
    
    def __mul__(self, other):  return self.multiply(other)
    def __rmul__(self, other): return self.multiply(other)
    def __imul__(self, other): 
        self = self.multiply(other)
        return self
    
    def __truediv__(self, other):  return self.divide(other)
    def __rtruediv__(self, other): return self.rhs_divide(other) # non-commutative
    def __itruediv__(self, other): 
        self = self.divide(other)
        return self
        
    def __floordiv__(self, other):  return self.floor_divide(other)
    def __rfloordiv__(self, other): return self.rhs_floor_divide(other) # non-commutative
    def __ifloordiv__(self, other): 
        self = self.floor_divide(other)
        return self
    
    def __mod__(self, other):  return self.mod(other)
    def __rmod__(self, other): return self.rhs_mod(other)
    def __imod__(self, other):
        self = self.mod(other)
        return self
    
    def __pow__(self, other):  return self.pow(other)
    def __rpow__(self, other): return self.rhs_pow(other)
    def __ipow__(self, other):
        self = self.pow(other)
        return self
    
    # comparison functions
    def __eq__(self, other):
        if isinstance(other, Custom): return self.data == other.data
        return self.data == other
    
    def __ne__(self, other):
        if isinstance(other, Custom): return self.data != other.data
        return self.data != other
    
    def __lt__(self, other):
        if isinstance(other, Custom): return self.data < other.data
        return self.data < other
    
    def __gt__(self, other):
        if isinstance(other, Custom): return self.data > other.data
        return self.data > other
    
    def __le__(self, other):
        if isinstance(other, Custom): return self.data <= other.data
        return self.data <= other
    
    def __ge__(self, other):
        if isinstance(other, Custom): return self.data >= other.data
        return self.data >= other
    
    # unary operators
    def __pos__(self): 
        return self