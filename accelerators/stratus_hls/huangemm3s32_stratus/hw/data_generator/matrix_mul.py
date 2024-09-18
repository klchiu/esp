import numpy as np
import random
rows = 2
loaded_cols = 4
cols = 1
a = np.zeros([rows, loaded_cols])
b = np.zeros([loaded_cols, cols])

list1 = [1, 2, 3, 4, 5, 6]

for i in range(len(a)):
    for j in range(len(a[0])):
        a[i, j] = random.choice(list1)

for i in range(len(b)):
    b[i, 0] = random.choice(list1)

ans = np.matmul(a, b)

print(a)
print(b)
print(ans)

