import socket
import select
import random
import time

if __name__ == '__main__':
    fd = open("FakeAccel.txt", "r+")
    fd.truncate(0)
    fd.close()

    val = 1.0
    n_points = 1
    timez = 0
    while True:
        with open("FakeAccel.txt", "a+") as writer:
            n_cyc = random.randint(1, 5)
            for ii in range(n_cyc):
                line = str(str(timez) + " " + str(val)[:6] + "\n")
                timez = timez + 3
                writer.write(line)
                val = val + random.uniform(-0.1, 0.1)
        time.sleep(1.0)
        
