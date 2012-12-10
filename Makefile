CC=gcc
LIB= -lm -lz -lpcre -lcrypto
PLATFORM = -DLINUX
TARGET=uws
OBJ=	uws.o uws_utils.o uws_socket.o uws_mime.o uws_config.o uws_router.o uws_fastcgi.o uws_cgi.o uws_http.o uws_index.o uws_header.o uws_fdhandler.o uws_datatype.o uws_error.o uws_rewrite.o uws_proxy.o uws_auth.o uws_memory.o uws_status.o

$(TARGET):	$(OBJ)
	$(CC) $(CFLAGS) $^ -o $@  $(LIB) -DDEBUG -g
release: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $(TARGET) $(LIB) -O2
test: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $(TARGET) $(LIB) -lusmem -DUSE_POOL -L. -O2
test-debug: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $(TARGET) $(LIB) -lusmem -DUSE_POOL -L. -DDEBUG -g

clean:
	-rm *.o uws
