import unittest

from sim import Vector
from sim import Shape


class TestVector(unittest.TestCase):

    def test_length(self):
        self.assertEqual(Vector(3, 4).length(), 5)

    def test_equals(self):
        self.assertEqual(Vector(1, 1), Vector(1, 1))

    def test_norm(self):
        self.assertEqual(Vector(0, 2).norm(), Vector(0, 1))

    def test_mult_vector(self):
        self.assertEqual(Vector(2, 3) * Vector(4, 5), Vector(8, 15))

    def test_mult_constant(self):
        self.assertEqual(Vector(2, 3) * 4, Vector(8, 12))

    def test_perp(self):
        self.assertEqual(Vector(3, 1).perp(), Vector(1, -3))

    def test_sub(self):
        self.assertEqual(Vector(4, 3) - Vector(2, 1), Vector(2, 2))

    def test_add(self):
        self.assertEqual(Vector(4, 3) + Vector(2, 1), Vector(6, 4))


class TestShape(unittest.TestCase):

    def test_centroid(self):
        shape = Shape([[0, 0], [0, 4], [2, 4], [2, 0]])
        self.assertEqual(shape.centroid, Vector(1, 2))

    def test_get_area(self):
        shape = Shape([[0, 1], [2, 0], [4, 1], [2, 2]])
        self.assertEqual(shape.get_area(), (4, Vector(2, 1)))
