# -*- coding: utf-8 -*-
"""
Created on Mon May 30 19:39:42 2022

@author: 86158
"""

import numpy as np
import matplotlib.pyplot as plt


def f(x, y):
    return 2 * x ** 3 - y ** 3 + x ** 2 + 2 * y ** 2 - 9 * x + 3 * y


def dfdx(x, y):
    return 6 * x ** 2 + 2 * x - 9


def dfdy(x, y):
    return - 3 * y ** 2 + 4 * y + 3


def output(times, x_0, y_0):
    print("迭代次数：", times)
    print("极值点：x_0:%f y_0:%f" % (x_0, y_0))
    print("极值：", f(x_0, y_0))


def Graph(result1, result2):
    x = np.arange(0, 2, 0.01)
    y = np.arange(-1, 2, 0.01)
    X, Y = np.meshgrid(x, y)
    plt.contour(X, Y, f(X, Y), 20)
    plt.plot(result1[:, 0], result1[:, 1], 'rv', result1[:, 0], result1[:, 1])
    plt.plot(result2[:, 0], result2[:, 1], 'b*', result2[:, 0], result2[:, 1])
    plt.show()


def GradientDescent(x=1.5, y=1.0, learn_rate=0.01, max_iters=1000, epsilon=1e-10):
    f_pre = f(x, y)
    f_cur = 0.0

    iTimes = 0

    sequence = np.array([x, y])

    while True:
        if iTimes > max_iters or abs(f_pre - f_cur) <= epsilon:
            break

        f_pre = f(x, y)

        gradient_x = dfdx(x, y)
        gradient_y = dfdy(x, y)

        x = x - learn_rate * gradient_x
        y = y - learn_rate * gradient_y

        f_cur = f(x, y)

        sequence = np.vstack((sequence, np.array([x, y])))
        iTimes += 1

    print("梯度下降法:")
    output(iTimes, x, y)
    return sequence


def NAG(x=1.5, y=1.0, learn_rate=0.01, max_iters=1000, epsilon=1e-10):
    f_pre = f(x, y)
    f_cur = 0.0

    iTimes = 0
    factor = 0.7

    momentum_x = 0.0
    momentum_y = 0.0
    sequence = np.array([x, y])

    while True:
        if iTimes > max_iters or abs(f_pre - f_cur) <= epsilon:
            break

        f_pre = f(x, y)

        gradient_x = dfdx(x - momentum_x, y - momentum_y)
        gradient_y = dfdy(x - momentum_x, y - momentum_y)

        momentum_x = factor * momentum_x + learn_rate * gradient_x
        momentum_y = factor * momentum_y + learn_rate * gradient_y

        x, y = x - momentum_x, y - momentum_y

        f_cur = f(x, y)

        sequence = np.vstack((sequence, np.array([x, y])))
        iTimes += 1

    print("NAG:")
    output(iTimes, x, y)
    return sequence


x = 1.5
y = 1.0
g1 = NAG(x, y)
g2 = GradientDescent(x, y)
Graph(g1, g2)