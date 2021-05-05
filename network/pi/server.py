#!/usr/bin/env python3

import bluetooth
import subprocess
import sys
import time
import io
try:
        subprocess.call(['sudo', 'hciconfig', 'hci0', 'piscan'])
except:
        print("???")
        sys.exit()

server_sock = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
server_sock.bind(("", bluetooth.PORT_ANY))
server_sock.listen(1)

port = server_sock.getsockname()[1]

uuid = "8ce255c0-200a-11e0-ac64-0800200c9a66"

bluetooth.advertise_service(server_sock, "SampleServer", service_id=uuid,
                            service_classes=[uuid, bluetooth.SERIAL_PORT_CLASS],
                            profiles=[bluetooth.SERIAL_PORT_PROFILE],
                            # protocols=[bluetooth.OBEX_UUID]
                            )

print("Waiting for connection on RFCOMM channel", port)

client_sock, client_info = server_sock.accept()
print("Accepted connection from", client_info)

while True:
    try:
        while True:
            data = client_sock.recv(1024)
            if not data:
                break
            code=str(data)[2]
            #print(data)
            if code=='1': # change settings
                with open('config.txt','r') as f:
                    settings=f.read()
                with open('config.txt','w') as f:
                    f.write(str(data)[3:])
                    
                print('changed settings to: ' + str(data)[4:-1])
                client_sock.send(str.encode('1'))
            elif code=='2': # requested file
                try:
                    with open("New","rb") as f: #add file called new to your directory to transfer it
                        print('opened file')
                        client_sock.send(str.encode(code))
                        print('sent opcode for sending file')
                        bytes=1
                        while bytes:
                            time.sleep(0.5)
                            #prevent sending data too quickly. Android can't handle it for some reason
                            #this just makes sure for every 1000 bytes sent, we get a response at least once
                            data = client_sock.recv(1024)
                            print('phone received data')
                            bytes=f.read(1000)
                            print(len(bytes))
                            client_sock.send(bytes)
                            print('sent')
                        print('finished sending')
                        client_sock.send(str.encode('\x04'))
                except OSError as err:
                        print("OS error: {0}".format(err))
                        client_sock.send(str.encode('3')) 
                #client_sock.send(str.encode(temp))
    except OSError as err:
        print("OS error: {0}".format(err))



client_sock.close()
server_sock.close()
print("All closed.")
