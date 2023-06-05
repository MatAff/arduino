from collections import deque
import cv2
import math
import numpy as np
import pandas as pd

PI = 3.14159


def deg_to_rad(deg):
    return deg / 360 * 2 * PI


def sin(deg):
    return math.sin(deg_to_rad(deg))


def cos(deg):
    return math.cos(deg_to_rad(deg))


def points_to_deg(start, end):
    # TODO: Implement more elegant approach
    dx = end[0] - start[0]
    dy = end[1] - start[1]
    # print(dx, dy)
    if dx == 0:
        if dy > 0:
            return 90
        else:
            return 270
    deg = math.atan(dy/dx) / 2 / PI * 360
    if dx < 0:
        deg = deg + 180
    if deg < 0:
        deg = deg + 360
    return deg

# points_to_deg(track.points[0], track.points[1])


segments = 5
segment_length = 50   #mm

delay = 50  # ms
cycle_speed = 0.5  # cycles per second
max_wheel_angle = 30.0  # degrees
cycle_proportion = 1.0 * 360  # part of complete wave

steps_per_second = 1000.0 / delay
step = cycle_speed / steps_per_second * 360  # degrees
wheel_step = cycle_proportion / segments  # phase difference between wheels
queue_pause = int(wheel_step / step)  # steps


class Track():

    def __init__(self, n=None):
        if n is None:
            n = 1000
        self.points = []
        point_count = n
        for i in range(point_count):
            x = 300 + sin(2*i/point_count * 360) * 100  # 2
            y = 300 + cos(1*i/point_count * 360) * 100  # 4
            self.points.append([x, y])
        self.pos = 0

    def tick(self):
        self.pos = self.pos + 1

    def angle(self):
        # TODO: Prevent flip
        start = self.points[self.pos]
        end = self.points[self.pos + 1]
        return points_to_deg(start, end)

    def diff(self):
        start = self.points[self.pos + queue_pause]
        end = self.points[self.pos + 1 + queue_pause]
        return self.angle() - math.atan((end[1]-start[1])/(end[0]-start[0])) / 2 / PI * 360

    def display(self, frame):
        for point in self.points:
            point = [int(e) for e in point]
            cv2.circle(frame, point, 1, (255, 0, 0), 1)
        return frame


# track = Track(36)

# for i in range(len(track.points) - 1):
#     print(track.angle())
#     track.tick()

# print(track.track)

# for i in range(len(track.points) - 1):
#     print(track.angle())
#     track.tick()


class Snake():

    def __init__(self):
        self.phase = 0
        self.wheel_angles = [-1] * 5
        self.joint_angles = list(range(segments - 1))
        self.wheel_height = list(range(segments))
        self.q = deque()
        self.log = []

    def __repr__(self):
        return ('joint_angles: ' + str([int(e) for e in self.joint_angles]) +
                '; wheel_angles: ' + str([int(e) for e in self.wheel_angles]))

    def calc_angles_queue(self, track):
        self.phase = (self.phase - step) % 360
        start = sin(self.phase) * max_wheel_angle
        end = sin(self.phase + wheel_step) * max_wheel_angle
        angle = end - start
        track.tick()
        diff = track.diff()
        row = {
            'start': start,
            'end': end,
            'angle': angle,
            'diff': diff,
        }
        angle = angle + diff
        row['angle_diff'] = angle
        start = start + track.angle()
        row['start_angle'] = start
        self.wheel_angles[0] = start
        row['wheel_angle'] = self.wheel_angles[0]
        self.q.append(angle)
        self.joint_angles = []
        for i in range(segments - 1):
            if len(self.q) > (i * queue_pause + 1):
                self.joint_angles.append(self.q[-1 * (i * queue_pause +1)])
                if i == (segments - 1):
                    self.q.popleft()
            else:
                self.joint_angles.append(360)
            row['joint_angle_' + str(i)] = self.joint_angles[-1]
        self.log.append(row)
        # if abs(self.joint_angles[0] - 360) > 90:
        #     print(self.joint_angles)
        #     print(self.wheel_angles)
        #     raise ValueError("Invalid angle")
        return self.joint_angles

    # def calc_angles(self):
    #     self.phase = (self.phase - step) % 360
    #     for i in range(segments):
    #         self.wheel_angles[i] = sin(self.phase + wheel_step * i) * max_wheel_angle
    #     self.joint_angles = []
    #     for start, end in zip(self.wheel_angles[:-1], self.wheel_angles[1:]):
    #         self.joint_angles.append(end - start)
    #     for i in range(segments):
    #         self.wheel_height[i] = cos(self.phase + wheel_step * i) * -max_wheel_angle
    #     return self.joint_angles

    def display(self, frame):

        # # Joints only
        # for i in range(segments -1):
        #     cv2.circle(frame, (i * 20 + 100, int(snake.joint_angles[i]) + 100), 5, (0, 255, 0), 5)

        # Level lines
        points = []
        dir = self.wheel_angles[0]
        x = 100
        y = 100 + self.wheel_height[0]
        points.append([x, y])
        x = x + cos(dir) * segment_length
        y = y + sin(dir) * segment_length
        points.append([x, y])
        for angle in self.joint_angles:
            dir = dir + angle
            x = x + cos(dir) * segment_length
            y = y + sin(dir) * segment_length
            points.append([x, y])
        for start, end in zip(points[:-1], points[1:]):
            start = [int(e) for e in start]
            end = [int(e) for e in end]
            cv2.line(frame, start, end, (0, 255, 0), 2)

        return frame


if __name__ == '__main__':

    track = Track()
    # print(track.track)

    snake = Snake()
    # snake.calc_angles()
    # print(snake.joint_angles)


    cv2.namedWindow('Sim')
    running = True
    count = 1
    while running:

        angles = snake.calc_angles_queue(track)

        frame = np.zeros((480, 640, 3), np.uint8)
        frame = snake.display(frame)
        frame = track.display(frame)

        cv2.imshow('Sim', frame)
        key=  cv2.waitKey(delay)

        if key != -1:
            print(key)
        if key == 115:
            delay = int(delay * 1.2)
        if key == 102:
            delay = int(delay / 1.2)

        count = count + 1
        if count > 500 or key == 27:
            running = False

    cv2.destroyAllWindows()

df = pd.DataFrame(snake.log)

for col in df.columns:
    df[col].plot()

df.columns
df['start'].plot()
df['end'].plot()
df['angle'].plot()
df['wheel_angle'].plot()
df['joint_angle_0'].plot()
