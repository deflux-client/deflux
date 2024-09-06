import socket

s = socket.socket()
s.connect(("127.0.0.1", 2347))

message = input("> ")
size = len(message)

s.send(size.to_bytes(2, 'big', signed=False))
s.send(message.encode())
data = s.recv(size + 2)

print(data)
