# ! python3 -m pip install opencv-python

import cv2
import numpy as np

DISPLAY_SHAPE = (480, 640, 3)
GRAVITY = 10
DELTA_TIME = 50
MS = (1 / 1000)
DEBUG = False


def overlap(source, target):
    return (source[1] > target[0]) & (source[0] < target[1])

class Vector:

    def __init__(self, x=0, y=0):
        self.x = x
        self.y = y
        self.P = [x, y]

    def length(self):
        return (self.x**2 + self.y**2)**0.5

    def norm(self):
        length = self.length()
        return Vector(self.x / length, self.y / length)

    def perp(self):
        return Vector(self.y, -self.x)

    def dot(self, other):
        return self.x * other.x + self.y * other.y

    def cross(self, vector):
        return self.x * vector.y - self.y * vector.x;

    def to_int_list(self):
        return [int(self.x), int(self.y)]

    def __repr__(self):
        return str({'x': self.x, 'y': self.y})

    def __mul__(self, obj):
        if isinstance(obj, Vector):
            return Vector(self.x * obj.x, self.y * obj.y)
        return Vector(self.x * obj, self.y * obj)

    def __truediv__(self, obj):
        if isinstance(obj, Vector):
            return Vector(self.x / obj.x, self.y / obj.y)
        return Vector(self.x / obj, self.y / obj)


    def __sub__(self, vector):
        return Vector(self.x - vector.x, self.y - vector.y)

    def __add__(self, vector):
        return Vector(self.x + vector.x, self.y + vector.y)

    def __pow__(self, e):
        return Vector(self.x**e, self.y**e)

    def __eq__(self, vector):
        return (self.x == vector.x) & (self.y == vector.y)


class Body:

    def __init__(self,position, mass, inertia, restitution, color, name):
        self.position = position
        self.velocity = Vector()
        self.acceleration = Vector()
        self.force = Vector()
        self.mass = mass
        self.name = name

        self.rotation = 0
        self.angular_velocity = 0
        self.angular_acceleration = 0
        self.torque = 0
        self.inertia = inertia

        self.friction = 0
        self.restitution = restitution
        self.color = color

    def update(self, delta_time):

        self.force = self.force + Vector(0, GRAVITY)

        # Position
        self.acceleration = self.force * (1 / self.mass)
        self.velocity = self.velocity + self.acceleration * delta_time * MS
        self.position = self.position + self.velocity * delta_time * MS

        # Rotation
        self.angular_acceleration = self.torque * (1 / self.inertia)
        self.angular_velocity = self.angular_acceleration * delta_time * MS
        self.rotation = self.angular_velocity * delta_time * MS

        # Zero forces
        self.force = Vector(0, 0)
        self.torque = 0

    def box_overlap(self, other):
        if overlap([self.get_box()[0].x, self.get_box()[1].x],
                   [other.get_box()[0].x, other.get_box()[1].x]):
            if overlap([self.get_box()[0].y, self.get_box()[1].y],
                       [other.get_box()[0].y, other.get_box()[1].y]):
                return True
            return False
        return False

class Circle(Body):

    def __init__(self, position, mass, radius, inertia, restitution, color, name):
        super().__init__(position, mass, inertia, restitution, color, name)
        self.radius = radius

    def get_box(self):
        return [Vector(-self.radius + self.position.x,
                       -self.radius  + self.position.y),
                Vector(self.radius + self.position.x,
                       self.radius + self.position.y)]

    def add_to_frame(self, frame):
        cv2.circle(frame, self.position.to_int_list(), self.radius,
                   self.color, 1)

    def collide(self, other):
        delta = self.position - other.position
        return (delta.x**2 + delta.y**2) < (self.radius + other.radius)**2


def resolve_collision(source, target):
    collision_normal = (source.position - target.position).norm()
    relative_velocity = source.velocity - target.velocity
    velocity_along_normal = relative_velocity.dot(collision_normal)
    if velocity_along_normal > 0:
        return
    restitution = min(source.restitution, target.restitution)
    impulse_scalar =  -(1 + restitution) * velocity_along_normal
    impulse_scalar = impulse_scalar / (1 / source.mass + 1 / target.mass)
    impulse = collision_normal * impulse_scalar
    source.velocity = source.velocity + impulse * (1 / source.mass)
    target.velocity = target.velocity - impulse * (1 / target.mass)


class World:

    def __init__(self):
        self.bodies = []
        self.time = 0

    def get_frame(self):
        frame = np.zeros(DISPLAY_SHAPE, np.uint8)
        for body in self.bodies:
            body.add_to_frame(frame)
        return frame

    def update(self, delta_time):
        for body in self.bodies:
            body.update(delta_time)
        self.time = self.time + delta_time
        if DEBUG:
            print(self.time)

    def detect_colistions(self):
        colisions = []
        for pos, source in enumerate(self.bodies[:-1]):
            for target in self.bodies[pos+1:]:
                if source.box_overlap(target):
                    #   print('overlap')
                    if source.collide(target):
                        colisions.append([source, target])
                        print('boom!', source.name, target.name)
                        resolve_collision(source, target)

    def handle_colisions(self):
        self.detect_colistions()


class Display:

    def __init__(self, name):
        self.name = name
        cv2.namedWindow(self.name)

    def imshow(self, frame):
        cv2.imshow(self.name, frame)
        key = cv2.waitKey(10)
        if key > 0:
            print(key)
        return key

    def close(self):
        cv2.destroyAllWindows()


if __name__ == '__main__':

    display = Display('Sim')

    # Initiate world
    world = World()
    left = Circle(Vector(100, 205), 10, 15, 5, 0.1, (0, 0, 255), 'red')
    left.velocity = Vector(10, 0)
    world.bodies.append(left)

    right = Circle(Vector(200, 195), 10, 15, 5, 0.2, (0, 255, 0), 'green')
    right.velocity = Vector(-10, 0)
    world.bodies.append(right)

    middle = Circle(Vector(150, 150), 10, 15, 5, 0.2, (255, 0, 0), 'blue')
    middle.velocity = Vector(0, 10)
    world.bodies.append(middle)

    # Main loop
    key = 0
    while key != 27:
        key = display.imshow(world.get_frame())
        world.update(DELTA_TIME)
        world.handle_colisions()

    display.close()
