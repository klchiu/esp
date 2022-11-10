#!/usr/bin/env python
# coding=utf-8

from ctypes import *
#import ctypes
import pathlib


if __name__ == "__main__":

    so_file = "./myfunctions.so"

    my_functions = CDLL(so_file)
    #my_functions = ctypes.CDLL(so_file)

    print(type(my_functions))

    j = my_functions.square(77)
    print(j)


    # Load the shared library into ctypes
    libname = pathlib.Path().absolute() / "libcmult.so"
    print(pathlib.Path().absolute())
    print(libname)
    c_lib = CDLL(libname)
    #c_lib = ctypes.CDLL(libname)

    x, y = 6, 2.3

    c_lib.cmult.restype = c_float
    #c_lib.cmult.restype = ctypes.c_float
    answer = c_lib.cmult(x, c_float(y))
    #answer = c_lib.cmult(x, ctypes.c_float(y))
    print(f"    In Python: int: {x} float {y:.1f} return val {answer:.1f}")
    print(answer)


