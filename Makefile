CC=gcc
CFLAGS= -g
uws:	uws.o uws_socket.o uws_mime.o uws_config.o uws_router.o uws_fastcgi.o uws_cgi.o uws_http.o uws_index.o uws_header.o

clean:
	-rm *.o uws
