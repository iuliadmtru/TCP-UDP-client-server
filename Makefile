CFLAGS = -Wall -g -Werror -Wno-error=unused-variable
LIBS = -lm

PORT = 12345
IP_SERVER = 127.0.0.1
ID_CLIENT = 1

server: server.c util/common.c util/helpers.c server_utils/util.c
	gcc $(CFLAGS) server.c util/common.c util/helpers.c server_utils/util.c -o server $(LIBS)

subscriber:

run_server:
	./server ${IP_SERVER} ${PORT}

run_server_valgrind:
	valgrind ./server ${IP_SERVER} ${PORT}

run_udp_client:
	python3 udp_client_test.py

.PHONY: clean
clean:
	rm server
