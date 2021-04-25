import socket
import select

if __name__ == '__main__':
    fs = open("sleep_config.txt", "r")
    name = fs.readline()
    ws = fs.readline()
    pm = fs.readline()
    interval = fs.readline()
    port = fs.readline()
    fs.close()

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind(('localhost', int(port)))
    sock.listen()
    print("Listen")
    con, addr = sock.accept()
    print("Accept")
    with con:
        print('Got: ', name, ' ', ws, ' ', pm, ' ', port, ' ', interval)
        while True:
            data = con.recv(256)
            print(str(data))
