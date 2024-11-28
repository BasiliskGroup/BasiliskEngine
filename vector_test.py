import glm

glm_vec3 = glm.vec3

class vec3():
    def __init__(self, x, y, z, callback=None):
        self.callback = callback
        self.data = glm.vec3(x, y, z)

    @property
    def x(self): return self.data.x
    @property
    def y(self): return self.data.y
    @property
    def z(self): return self.data.z

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

    def __repr__(self):
        return str(self.data)

    def __iter__(self):
        return iter((self.x, self.y, self.z))
    
    def __glm_vec3__(self):
        return glm.vec3(self.x, self.y, self.z)
    
def notify():
    print("changed a value")


test_vec = vec3(3, 5, 6, callback=notify)

test_vec.x = 4

print(glm.normalize(test_vec))