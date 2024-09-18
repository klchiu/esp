import numpy as np
import random
# rows = 10
# loaded_cols = 32678
# cols = 1

rows = 2
loaded_cols = 4
cols = 1

# rows = 10
# loaded_cols = 300
# cols = 1

a = np.zeros([rows, loaded_cols])
b = np.zeros([loaded_cols, cols])

list1 = [1, 2, 3, -1, -2, -3]

for i in range(len(a)):
    for j in range(len(a[0])):
        a[i, j] = random.choice(list1)

for i in range(len(b)):
    b[i, 0] = random.choice(list1)

ans = np.matmul(a, b)

print(a)
print(b)
print(ans)

# generate some files
with open('init_data.hpp', 'w') as f:
    f.write("#ifndef __INIT_DATA_HPP__\n")
    f.write("#define __INIT_DATA_HPP__\n")
    f.write("\n")
    f.write("#include \"system.hpp\"\n")
    f.write("\n")
    f.write("void data_init(FPDATA* inA, FPDATA* inB, FPDATA* gold) {\n")

    # a needs to be transposed
    for j in range(len(a[0])):
        for i in range(len(a)):
            f.write(f"    inA[{j*len(a)+i}] = {a[i, j]};\n")

    for j in range(len(b)):
        f.write(f"    inB[{j}] = {b[j, 0]};\n")

    for i in range(rows):
        for j in range(cols):
            f.write(f"    gold[{i*cols+j}] = {ans[i, j]};\n")

    f.write("}\n")
    f.write("#endif\n")

# with open('../tb/init_data.hpp', 'w') as f:
#     f.write("#ifndef __INIT_DATA_HPP__\n")
#     f.write("#define __INIT_DATA_HPP__\n")
#     f.write("\n")
#     f.write("#include \"system.hpp\"\n")
#     f.write("\n")
#     f.write("void data_init(FPDATA* inA, FPDATA* inB, FPDATA* gold) {\n")

#     # a needs to be transposed
#     for j in range(len(a[0])):
#         for i in range(len(a)):
#             f.write(f"    inA[{j*len(a)+i}] = {a[i, j]};\n")

#     for j in range(len(b)):
#         f.write(f"    inB[{j}] = {b[j, 0]};\n")

#     for i in range(rows):
#         for j in range(cols):
#             f.write(f"    gold[{i*cols+j}] = {ans[i, j]};\n")

#     f.write("}\n")
#     f.write("#endif\n")
print("data is in init_data.hpp")
        