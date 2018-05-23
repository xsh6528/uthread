def f(i):
    return i * 1.07

def g(i):
    return (i + 72000) * 1.07

i = 0
for j in range(30):
    i = g(i)
print(i)
