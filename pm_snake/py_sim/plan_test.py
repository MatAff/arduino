import unittest

from plan import points_to_deg


class TestPointsToDeg(unittest.TestCase):

    def test_north(self):
        start = [0, 0]
        end = [0, 1]
        self.assertEqual(points_to_deg(start, end), 90)

    def test_north_west(self):
        start = [0, 0]
        end = [-1, 1]
        self.assertAlmostEqual(points_to_deg(start, end), 135, 3)

    def test_west(self):
        start = [0, 0]
        end = [-1, 0]
        self.assertAlmostEqual(points_to_deg(start, end), 180, 3)

    def test_south_west(self):
        start = [0, 0]
        end = [-1, -1]
        self.assertAlmostEqual(points_to_deg(start, end), 225, 3)

    def test_south(self):
        start = [0, 0]
        end = [0, -1]
        self.assertAlmostEqual(points_to_deg(start, end), 270, 3)

    def test_south_east(self):
        start = [0, 0]
        end = [1, -1]
        self.assertAlmostEqual(points_to_deg(start, end), -45, 3)

    def test_east(self):
        start = [0, 0]
        end = [1, 0]
        self.assertAlmostEqual(points_to_deg(start, end), 0, 3)

    def test_north_east(self):
        start = [0, 0]
        end = [1, 1]
        self.assertAlmostEqual(points_to_deg(start, end), 45, 3)
