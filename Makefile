CC=gcc
LIB= -lm -lz -lpcre -lcrypto -ltcmalloc
TARGET=uws
OBJ=	uws.o uws_socket.o uws_mime.o uws_config.o uws_router.o uws_fastcgi.o uws_cgi.o uws_http.o uws_index.o uws_header.o uws_utils.o uws_fdhandler.o uws_datatype.o uws_error.o uws_rewrite.o uws_proxy.o uws_auth.o uws_memory.o

$(TARGET):CFLAGS=-DDEBUG -g
release:CFLAGS=-O3

$(TARGET):	$(OBJ)
	$(CC) $(CFLAGS) $^ -o $@  $(LIB)
release: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $(TARGET) $(LIB)

clean:
	-rm *.o uws
