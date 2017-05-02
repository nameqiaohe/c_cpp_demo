SERVER_SRC = server.c
CLIENT_SRC = client.c

COMMON = aes_options.c

LIBS = -lcrypto

all:
	gcc -g -o server $(SERVER_SRC) $(COMMON) $(LIBS)
	gcc -g -o client $(CLIENT_SRC) $(COMMON) $(LIBS)

.PHONY:clean
clean:
	rm server client
