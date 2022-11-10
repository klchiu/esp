CC=gcc
#GCC=riscv64-unknown-linux-gnu-gcc

all: so test_python

so:
	$(CC) -fPIC -shared -o myfunctions.so myfunctions.c
	$(CC) -fPIC -shared -o libcmult.so cmult.c

test_python:
	python3 test_ctypes.py

clean:
	rm *.so

.PHONY: all so test_python clean
