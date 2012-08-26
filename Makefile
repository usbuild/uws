CC=gcc
CFLAGS= -g -pthread
TARGET=uws
OBJ=	uws.o uws_socket.o uws_mime.o uws_config.o uws_router.o uws_fastcgi.o uws_cgi.o uws_http.o uws_index.o uws_header.o uws_utils.o uws_fdhandler.o uws_datatype.o uws_error.o
$(TARGET):	$(OBJ)
		$(CC) $(CFLAGS)  $^ -o $@ -lm

clean:
	-rm *.o uws
