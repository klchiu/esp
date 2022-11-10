#!/usr/bin/env python
# coding=utf-8

#from ctypes import *
import ctypes
import pathlib


if __name__ == "__main__":
    # Load the shared library into ctypes
    libesp_so = pathlib.Path().absolute() / "libesp.so"
    print(pathlib.Path().absolute())
    print(libesp_so)
    lib_esp = ctypes.CDLL(libesp_so)

 #   c_libesp.esp_thread_info_t cfg

    print("test1")
    x = lib_esp.esp_dummy(9)
    print("test2")
    lib_esp.esp_py_run(2, 1)
    print(x)

