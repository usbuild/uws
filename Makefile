CC=gcc
LIB= -lm -lz -lpcre -lcrypto
PLATFORM = -DLINUX
TARGET=uws
OPT_DEBUG= -DDEBUG -g
OPT_RELEASE= -O2
OPT_USEPOOL=-L. -lusmem -DUSE_POOL
OBJ=	uws.o uws_utils.o uws_socket.o uws_mime.o uws_config.o uws_router.o uws_fastcgi.o uws_cgi.o uws_http.o uws_index.o uws_header.o uws_fdhandler.o uws_datatype.o uws_rewrite.o uws_proxy.o uws_auth.o uws_memory.o uws_status.o uws_handlers.o

$(TARGET):CFLAGS=$(OPT_DEBUG)
release:CFLAGS=$(OPT_RELEASE)
test: CFLAGS= $(OPT_DEBUG)
test-release: CFLAGS= $(OPT_RELEASE)


$(TARGET):	$(OBJ)
	$(CC) $(CFLAGS) $^ -o $@  $(LIB)
release: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $(TARGET) $(LIB)
test: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $(TARGET) $(LIB) $(OPT_USEPOOL) $(OPT_DEBUG)
test-release: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $(TARGET) $(LIB) $(OPT_USEPOOL) $(OPT_RELEASE)

clean:
	-rm *.o uws
