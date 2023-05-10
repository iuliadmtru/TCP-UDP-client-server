CFLAGS = -Wall -g -Werror -Wno-error=unused-variable
SERV_SOURCES = server.c poller.c UDP_server.c TCP_server.c clients.c util.c subscriptions.c
CLI_SOURCES = TCP_client.c poller.c util.c
LIBS =

PORT = 12345
IP_SERVER = 127.0.0.1
ID_CLIENT = client1

all: server subscriber

server: $(SERV_SOURCES)
	gcc $(CFLAGS) $(SERV_SOURCES) -o server $(LIBS)

subscriber:
	gcc $(CFLAGS) $(CLI_SOURCES) -o subscriber $(LIBS)

clients_tests:
	gcc $(CFLAGS) clients.c clients_tests.c -o clients_tests


run_server:
	./server ${PORT}

run_server_valgrind:
	valgrind --track-origins=yes ./server ${PORT}

run_subscriber:
	./subscriber ${ID_CLIENT} ${IP_SERVER} ${PORT}

run_subscriber_valgrind:
	valgrind --track-origins=yes ./subscriber ${ID_CLIENT} ${IP_SERVER} ${PORT}

run_udp_client:
	python3 udp_client_test.py

run_clients_tests:
	valgrind --track-origins=yes ./clients_tests


.PHONY: clean
clean:
	rm server subscriber
