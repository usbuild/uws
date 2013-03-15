CC=gcc
LIB= -lm -lz -lpcre -lcrypto
PLATFORM = -DLINUX
TARGET=uws
OPT_DEBUG= -DDEBUG -g
OPT_RELEASE= -O2
OPT_USEPOOL=-L. -lusmem -DUSE_POOL
OBJ=	uws.c uws_utils.c uws_socket.c uws_mime.c uws_config.c uws_router.c uws_fastcgi.c uws_cgi.c uws_http.c uws_index.c uws_header.c uws_fdhandler.c uws_datatype.c uws_rewrite.c uws_proxy.c uws_auth.c uws_memory.c uws_status.c uws_handlers.c

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
	-rm uws uws_config.c
