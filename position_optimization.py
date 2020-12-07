import math
import matplotlib.pyplot as plt
import numpy as np

origin_x = 0
origin_y = 0

rotate_front_x_range = np.arange(-0.5, -1, -0.5)

# settings

angle_range = np.arange(270.0, 180.0, -10.0)

base_angle_start = 180.0


# dev
rotate_front_x = -0.5
rotate_front_y = -0.5
front_len = 5.0
base_len = 5.0
rotate_back_x = -1.5
rotate_back_y = -1.0
angle = 270.0
base_angle = base_angle_start

### HELPER FUNCTIONS ###

def rad(deg):
    return deg / 180.0 * math.pi

def deg(rad):
    return rad / math.pi * 180.0

def calc_pos(angle, len, x, y):
    nx = math.cos(rad(angle)) * len + x
    ny = math.sin(rad(angle)) * len + y
    return nx, ny

def calc_len(xs, ys, xe, ye):
    return math.sqrt((xs - xe)**2 + (ys - ye)**2)

def calc_pos_len(ls, xs, ys, le, xe, ye):
    """Return position of point based on distance two other points"""
    G = -1 * (xs**2 + ys**2 - ls**2  - xe**2 - ye**2 + le**2) / (- 2*ys + 2*ye)
    H = -1 * (- 2*xs + 2*xe) / (- 2*ys + 2*ye)
    A = (1 + H**2)
    B = (-2*xs + 2*G*H - 2*H*ys)
    C = (xs**2 + G**2   - 2*G*ys  + ys**2 - ls**2)
    xp1 = (-B + math.sqrt(B**2-4*A*C)) / (2*A)
    xp2 = (-B - math.sqrt(B**2-4*A*C)) / (2*A)

    def get_yp(xp):
        yp = math.sqrt(ls**2  - (xp - xs)**2) + ys
        return yp

    yp1 = get_yp(xp1)
    yp2 = get_yp(xp2)

    return (xp1, yp1), (xp2, yp2)

### FUNCTIONS ###

def calc_base_front_pos(angle, front_len, rotate_front_x, rotate_front_y):
    return calc_pos(angle, front_len, rotate_front_x, rotate_front_y)

def calc_base_back_pos(base_angle, base_len, base_front_x, base_front_y):
    return calc_pos(base_angle, base_len, base_front_x, base_front_y)

def calc_back_len(base_back_x, base_back_y, rotate_back_x, rotate_back_y):
    return calc_len(base_back_x, base_back_y, rotate_back_x, rotate_back_y)

def calc_back_pos_dist(base_len, base_front_x, base_front_y, back_len, rotate_back_x, rotate_back_y):
    return calc_pos_len(base_len, base_front_x, base_front_y, back_len, rotate_back_x, rotate_back_y)

def calc_back_length(angle, front_len, rotate_front_x, rotate_front_y, base_angle, base_len):
    base_front_x, base_front_y = calc_base_front_pos(angle, front_len, rotate_front_x, rotate_front_y)
    base_back_x, base_back_y = calc_base_back_pos(base_angle, base_len, base_front_x, base_front_y)
    back_len = calc_back_len(base_back_x, base_back_y, rotate_back_x, rotate_back_y)
    return back_len

def calc_pos_on_angle(angle, back_len):
    # TODO: add missing parameters
    base_front_x, base_front_y = calc_base_front_pos(angle, front_len, rotate_front_x, rotate_front_y)
    back_opt = calc_back_pos_dist(base_len, base_front_x, base_front_y, back_len, rotate_back_x, rotate_back_y)
    if back_opt[0][0] < back_opt[1][0]:
        base_back_x, base_back_y = back_opt[0]
    else:
        base_back_x, base_back_y = back_opt[1]
    return base_front_x, base_front_y, base_back_x, base_back_y

def plot_gear(rotate_front_x, rotate_front_y, base_front_x, base_front_y,
              base_back_x, base_back_y, rotate_back_x, rotate_back_y):
    plt.plot([0, rotate_front_x, base_front_x, base_back_x, rotate_back_x],
            [0, rotate_front_y, base_front_y, base_back_y, rotate_back_y])

# determine back length
back_len = calc_back_length(angle, front_len, rotate_front_x, rotate_front_y, base_angle, base_len)

# update angle
angle = angle_range[1]

# update back pos
base_front_x, base_front_y, base_back_x, base_back_y = calc_pos_on_angle(angle, back_len)

# visualize angle
plot_gear(rotate_front_x, rotate_front_y, base_front_x, base_front_y,
              base_back_x, base_back_y, rotate_back_x, rotate_back_y)


# loop through angle
for angle in angle_range:
    print(angle)

    # update back pos
    base_front_x, base_front_y, base_back_x, base_back_y = calc_pos_on_angle(angle, back_len)

    # plot
    plot_gear(rotate_front_x, rotate_front_y, base_front_x, base_front_y,
                base_back_x, base_back_y, rotate_back_x, rotate_back_y)



# determine safe passage


# create cost function


# loop through range of settings and determine optimum




# ### DERIVATION ### 

# # based on https://math.stackexchange.com/questions/543961/determine-third-point-of-triangle-when-two-points-and-all-sides-are-known/544025#544025?newreg=856784ad81104a7ca9ce8bf29f8268df

# # def calc_pos_len(ls, xs, ys, le, xe, ye):

# # base equasiona
# (xp - xs)**2 + (yp - ys)**2 = ls**2
# (xp - xe)**2 + (yp - ye)**2 = le**2

# # expand
# xp**2 - 2*xp*xs + xs**2 + yp**2 - 2*yp*ys + ys**2 - ls**2 = 0
# xp**2 - 2*xp*xe + xe**2 + yp**2 - 2*yp*ye + ye**2 - le**2 = 0

# # substract
# - 2*xp*xs + xs**2 - 2*yp*ys + ys**2 - ls**2 + 2*xp*xe - xe**2 + 2*yp*ye - ye**2 + le**2 = 0

# # reorder
# - 2*xp*xs + 2*xp*xe - 2*yp*ys + 2*yp*ye + xs**2 + ys**2 - ls**2  - xe**2 - ye**2 + le**2 = 0
# (- 2*xp*xs + 2*xp*xe) + (- 2*yp*ys + 2*yp*ye) + (xs**2 + ys**2 - ls**2  - xe**2 - ye**2 + le**2) = 0

# # consolidate
# (- 2*xs + 2*xe) * xp + 
# (- 2*ys + 2*ye) * yp + 
# (xs**2 + ys**2 - ls**2  - xe**2 - ye**2 + le**2) = 0

# # define constants
# D = (- 2*xs + 2*xe)
# E = (- 2*ys + 2*ye)
# F = (xs**2 + ys**2 - ls**2  - xe**2 - ye**2 + le**2)
# D * xp + E * yp + F = 0

# # express one as the other
# yp = (- F - (D * xp)) / E
# yp = -F / E - (D/E * xp)

# # define constants
# G = -1 * (xs**2 + ys**2 - ls**2  - xe**2 - ye**2 + le**2) / (- 2*ys + 2*ye)
# H = -1 * (- 2*xs + 2*xe) / (- 2*ys + 2*ye)
# yp = G + H *xp

# # replace one with the other
# xp**2 - 2*xp*xs + xs**2 + (G + H *xp)**2 - 2*(G + H *xp)*ys + ys**2 - ls**2 = 0

# # expand
# xp**2 - 2*xp*xs + xs**2 + G**2 + 2*G*H*xp + H**2 * xp**2 - 2*G*ys - 2*H*xp*ys + ys**2 - ls**2 = 0

# # consolidate
# (xp**2 + H**2 * xp**2) + (-2*xp*xs + 2*G*H*xp - 2*H*xp*ys) + (xs**2 + G**2   - 2*G*ys  + ys**2 - ls**2) = 0
# (1 + H**2) * xp**2 + (-2*xs + 2*G*H - 2*H*ys) * xp + (xs**2 + G**2   - 2*G*ys  + ys**2 - ls**2) = 0

# # define abc
# A = (1 + H**2)
# B = (-2*xs + 2*G*H - 2*H*ys)
# C = (xs**2 + G**2   - 2*G*ys  + ys**2 - ls**2)

# # apply abc (-b±√(b²-4ac))/(2a) 
# xp1 = (B + math.sqrt(B**2-4*A*C)) / (2*A)
# xp2 = (B - math.sqrt(B**2-4*A*C)) / (2*A)
