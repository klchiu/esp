# General Matrix-Multiplication Accelerator (GeMM)


Parameters:
- ninputs: Number of inputs
- d1: Number of rows of the matrix 1
- d2: Number of cols of the matrix 1 (= Number of rows of the matrix 2)
- d3: Number of cols of the matrix 2
- ld_offset1: Input offset (matrix 1)
- ld_offset2: Input offset (matrix 2)
- st_offset: Output offset
- do_relu: Do ReLU stage
- transpose: True if matrix 2 is transposed