import numpy as np

SEGMENT_LENGTH = 100  # mm
SEGMENT_WIDTH = 50  # mm


class Vector:

    def __init__(self, x=0, y=0):
        self.x = x
        self.y = y
        self.P = [x, y]

    def length(self):
        print(self.x**2 + self.y**2)
        return (self.x**2 + self.y**2)**0.5

    def __eq__(self, vector):
        return (self.x == vector.x) & (self.y == vector.y)

    def norm(self):
        length = self.length()
        return Vector(self.x / length, self.y / length)

    def __mul__(self, obj):
        if isinstance(obj, Vector):
            return Vector(self.x * obj.x, self.y * obj.y)
        return Vector(self.x * obj, self.y * obj)

    def perp(self):
        return Vector(self.y, -self.x)

    def __repr__(self):
        return str({'x': self.x, 'y': self.y})

    def cross(self, vector):
        return self.x * vector.y - self.y * vector.x;

    def __sub__(self, vector):
        return Vector(self.x - vector.x, self.y - vector.y)

    def __add__(self, vector):
        return Vector(self.x + vector.x, self.y + vector.y)


class Shape:

    def __init__(self, vertices):
        self.vertices = vertices
        self.centroid = self.get_centroid()

    def get_centroid(self):
        t = list(zip(*self.vertices))
        return Vector(np.mean(t[0]), np.mean(t[1]))

    def get_area(self):
        """Compute total area and the center of mass."""
        self.total_area = 0
        self.center_mass = Vector()
        for pos, vertice in enumerate(self.vertices):
            next_vertice = self.vertices[pos + 1 - len(self.vertices)]
            base_edge = Vector(*vertice) - Vector(*next_vertice)
            center_edge = Vector(*next_vertice) - self.centroid
            area = 0.5 * np.abs(base_edge.cross(center_edge))
            vertices = [vertice, next_vertice, self.centroid.P]
            t = list(zip(*vertices))
            area_mass = Vector(np.mean(t[0]), np.mean(t[1]))
            combined = self.total_area + area
            self.center_mass = self.center_mass * (self.total_area / combined) + area_mass * (area / combined)
            self.total_area = self.total_area + area
        return self.total_area, self.center_mass


class Body:

    def __init__(self):
        self.position = Vector()
        self.velocity = Vector()
        self.acceleration = Vector()
        self.force = Vector()
        self.mass = 0

        self.rotation = 0
        self.angular_velocity = 0
        self.angular_acceleration = 0
        self.torque = 0
        self.inertia = 0

        self.friction = 0
        self.restitution = 0
        self.shape = Shape()

    def update(self, delta_time):

        # Position
        self.acceleration = self.force / self.mass
        self.velocity = self.velocity + self.acceleration * delta_time
        self.position = self.position + self.velocity * delta_time

        # Rotation
        self.angular_acceleration = self.torque / self.inertia
        self.angular_velocity = self.angular_acceleration * delta_time
        self.rotation = self.angular_velocity * delta_time

        # Zero forces
        self.force = Vector()
        self.torque = 0


class World:

    def __init__(self):
        self.bodies = []

    def update(self, delta_time):
        for body in self.bodies:
            body.update(delta_time)

    def display(self):
        pass  # TODO(maffourtit): implement


if __name__ == '__main__':

    # Initiate world
    world = World()

    # Add a body
    head = Body()
    shape = Shape([
        [0, 0],
        [SEGMENT_LENGTH / 2, SEGMENT_WIDTH / 2],
        [SEGMENT_LENGTH, 0],
        [SEGMENT_LENGTH / 2, -SEGMENT_WIDTH / 2],
    ])
    head.shape = shape

vertices = [[1, 2], [3, 4], [5, 6]]
t = list(zip(*vertices))
t
Vector(np.mean(t[0]), np.mean(t[1]))
