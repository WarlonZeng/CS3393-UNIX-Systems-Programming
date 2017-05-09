FLAGS = -std=c99 -Wall -O1

all: chat

chat: chat_server.c chat_client.c
	gcc -pthread ${FLAGS} -o chat_server chat_server.c
	gcc -pthread ${FLAGS} -o chat_client chat_client.c

clean:
	rm -f chat_server chat_client