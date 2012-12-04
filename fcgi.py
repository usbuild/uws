#!/usr/bin/env python
if __name__ == '__main__':  
    import socket  
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  
    sock.bind(('localhost', 9000))  
    sock.listen(5)  
    while True:  
        connection,address = sock.accept()  
        try:  
            connection.settimeout(5)  
            buf = connection.recv(1024)  
            print buf,
        except socket.timeout:  
            print 'time out'  
        connection.close()
