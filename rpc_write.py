import socket

s = socket.socket()
s.connect(("127.0.0.1", 2347))
size = 5

s.send(size.to_bytes(2, 'big', signed=False))
s.send(b't')
input("Press enter to send the rest")
s.send(b'est\n')
