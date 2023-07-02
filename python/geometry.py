import math

class vec3:
    def __init__(self, x: float, y: float, z: float):
        self.x = x
        self.y = y
        self.z = z

    # dot product
    def __mul__(self, other):
        return (self.x * other.x + self.y * other.y + self.z * other.z)
    
    def __add__(self, other):
        return vec3(self.x + other.x, self.y + other.y, self.z + other.z)
    
    def __sub__(self, other):
        return vec3(self.x - other.x, self.y - other.y, self.z - other.z)
    
    def __eq__(self, __value: object) -> bool:
        return (self.x == __value.x and self.y == __value.y and self.z == __value.z)

    def __repr__(self) -> str:
        return f"({self.x},{self.y},{self.z})"
    
    def normalize(self):
        norm = math.sqrt(self.x ** 2 + self.y ** 2 + self.z ** 2)
        return vec3(self.x/norm, self.y/norm, self.z/norm)

class Sphere:
    def __init__(self, center: vec3, radius: float):
        self.center = center
        self.radius = radius
    
    # used in ray intersection algo
    def discriminant(self, a: float, b: float, c: float):
        return (b * b - (4 * a * c))


    def ray_intersect(self, p: vec3, dir: vec3, t0: float):
        # geometric solution
        # https://www.lighthouse3d.com/tutorials/maths/ray-sphere-intersection/
        L = self.center - p
        tca = L * dir
        d2 = L*L - tca*tca
        if d2 > self.radius ** 2:
            return False
        thc = math.sqrt(self.radius ** 2 - d2)
        t0 = tca - thc
        t1 = tca + thc
        if (t0 < 0):
            t0 = t1       
        return t0 >= 0

        # analytic solution
        # L = center - p
        # a = dir * dir
        # b = vec3(2, 2, 2) * (dir * L) # until multiplication by a scalar is implemented 
        # c = (L * L) - (radius ** 2)
        # return discriminant(a, b, c) >= 0
    




class Ray:
    def __init__(self, origin: vec3, direction: vec3):
        self.origin = origin
        self.direction = direction
