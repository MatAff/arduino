import unittest

from sim import Circle
from sim import Vector
from sim import overlap


class TestVector(unittest.TestCase):

    def test_length(self):
        self.assertEqual(Vector(3, 4).length(), 5)

    def test_norm(self):
        self.assertEqual(Vector(0, 2).norm(), Vector(0, 1))

    def test_dot(self):
        self.assertEqual(Vector(1, 2).dot(Vector(3, 4)), 11)

    def test_to_int_list(self):
        self.assertEqual(Vector(1, 2).to_int_list(), [1, 2])

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

    def test_pow(self):
        self.assertEqual(Vector(2, 3)**2, Vector(4, 9))

    def test_equals(self):
        self.assertEqual(Vector(1, 1), Vector(1, 1))


class TestCircle(unittest.TestCase):

    def test_get_box(self):
        circle = Circle(Vector(0, 0), 5, 5, 5, 0.2, (0, 0, 255))
        self.assertEqual(circle.get_box(), [Vector(-5, -5), Vector(5, 5)])

class TestOverall(unittest.TestCase):

    def test_overlap_lower(self):
        self.assertTrue(overlap([1, 4], [0, 3]))

    def test_overlap_middle(self):
        self.assertTrue(overlap([1, 4], [2, 3]))

    def test_overlap_higher(self):
        self.assertTrue(overlap([1, 4], [2, 5]))

    def test_overlap_too_low(self):
        self.assertFalse(overlap([1, 4], [0, 0]))

    def test_overlap_too_high(self):
        self.assertFalse(overlap([1, 4], [5, 5]))


# SEGMENT_LENGTH = 100  # mm
# SEGMENT_WIDTH = 50  # mm
# POINT_TEMPLATE = [
#     [0, 0],
#     [SEGMENT_LENGTH / 2, SEGMENT_WIDTH / 2],
#     [SEGMENT_LENGTH, 0],
#     [SEGMENT_LENGTH / 2, -SEGMENT_WIDTH / 2],
# ]

# from sim import Polygon

# class TestPolygon(unittest.TestCase):

#     def test_get_box(self):
#         polygon = Polygon([[0, 0], [0, 4], [2, 4], [2, 0]])
#         self.assertEqual(polygon.get_box(), [Vector(0, 0), Vector(2, 4)])


#     def test_centroid(self):
#         polygon = Polygon([[0, 0], [0, 4], [2, 4], [2, 0]])
#         self.assertEqual(polygon.centroid, Vector(1, 2))

#     def test_get_area(self):
#         polygon = Polygon([[0, 1], [2, 0], [4, 1], [2, 2]])
#         self.assertEqual(polygon.get_area(), (4, Vector(2, 1)))


# class Polygon(Shape):

#     def __init__(self, vertices):
#         self.vertices = vertices
#         self.transpose_vertices = list(zip(*self.vertices))
#         self.centroid = self.get_centroid()

#     def get_box(self):
#         return [Vector(np.min(self.transpose_vertices[0]),
#                        np.min(self.transpose_vertices[1])),
#                 Vector(np.max(self.transpose_vertices[0]),
#                        np.max(self.transpose_vertices[1]))]

#     def get_centroid(self):
#         return Vector(np.mean(self.transpose_vertices[0]),
#                       np.mean(self.transpose_vertices[1]))

#     def get_area(self):
#         """Compute total area and the center of mass."""
#         self.total_area = 0
#         self.center_mass = Vector()
#         for pos, vertice in enumerate(self.vertices):
#             next_vertice = self.vertices[pos + 1 - len(self.vertices)]
#             base_edge = Vector(*vertice) - Vector(*next_vertice)
#             center_edge = Vector(*next_vertice) - self.centroid
#             area = 0.5 * np.abs(base_edge.cross(center_edge))
#             vertices = [vertice, next_vertice, self.centroid.P]
#             t = list(zip(*vertices))
#             area_mass = Vector(np.mean(t[0]), np.mean(t[1]))
#             combined = self.total_area + area
#             self.center_mass = (self.center_mass * (self.total_area / combined) +
#                 area_mass * (area / combined))
#             self.total_area = self.total_area + area
#         return self.total_area, self.center_mass

# Body(Vector(segment_number * SEGMENT_LENGTH), POINT_TEMPLATE))