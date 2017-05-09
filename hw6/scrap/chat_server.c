#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h> 
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <errno.h>

#define BUF_LEN 1024
#define PORT_NO 20000
#define USERNAME "SERVER"
#define MAX_CLIENTS 100
#define MAX_MESSAGES 100

/* create client data structure */
typedef struct {
	int client_sockfd;
	int client_pos;
	char client_username[16];
} client_t;
client_t *clients[MAX_CLIENTS]; /* clients */

/* create simple message queue */
char *messages[MAX_MESSAGES]; /* array of strings */

/* globals */
int ERR_JOIN = 1;
int ERR_LEAVE = 2;
int connected = 0;
int queued = 0;
// static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
// static pthread_mutex_t connected_mutex = PTHREAD_MUTEX_INITIALIZER;
// static pthread_mutex_t queued_mutex = PTHREAD_MUTEX_INITIALIZER;
// static pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
// static pthread_mutex_t messages_mutex = PTHREAD_MUTEX_INITIALIZER;

/* error messages */
char *SERVER_FULL = "Server is full, try again later\r\n";
char *SERVER_START = "Server started";
char *WELCOME_MSG = "Welcome to the chat server!\r\n";

/* program functions */
int parseArgs(char *argv[], char **username, int *port);
void clientAddQueue(client_t *client);
void clientDeleteQueue(client_t *client);
void messageAddQueue(char *message);
void flushMessageQueue();
void broadcastCurrentUsersToUser(client_t *client);
void logAndBroadcastJoinOrLeave(client_t *client, int err_code);
void *consumerThread(void *args);
void *clientCloneFactory(void* args);

/* parse arguments with a switch-case.. much more organized */
int parseArgs(char *argv[], char **username, int *port) {
	char* endptr;
	for (int i = 1; argv[i] != NULL; i++) {
		if (argv[i][0] != '-') {
			continue;
		}
		else {
			switch(argv[i][1]) {
				case 'p':
					*port = strtol(argv[i + 1], &endptr, 10);
					break;
				case 'u':
					*username = argv[i + 1];
					break;
				case 'h': default: 
					printf("Usage: ./chat_server -u USERNAME -p PORT_NO");
					break;
			}
		}
	}
	return 0;
}

/* add client to queue.. which is more like an array */
void clientAddQueue(client_t *client) {

	for (int i = 0; i < MAX_CLIENTS; i++) {
		if (!clients[i]) {
			clients[i] = client;
			client->client_pos = i;
			break;
		}
	}

}

/* delete client from queue.. an array structure */
void clientDeleteQueue(client_t *client) {

	clients[client->client_pos] = NULL;

}

/* add only client messages to queue, similar structure to clients */
void messageAddQueue(char *message) {

	for (int i = 0; i < MAX_MESSAGES; i++) {
		if (!messages[i]) {
			messages[i] = message;
			break;
		}
	}

}

/* send messages in queue to valid clients. */
void flushMessageQueue() {

	for (int i = 0; i < MAX_MESSAGES; i++) { /* for each message */
		if (messages[i]) {
			for (int j = 0; j < MAX_CLIENTS; j++) { /* for each client */
				if (clients[j]) {
					if (write(clients[j]->client_sockfd, messages[i], strlen(messages[i])) < 0) { /* send message to client */
						perror("Error: Failed to write to client");
					}
				}
			}
			messages[i] = NULL; /* erase message */
		}
	}

}

/* create a c-string which includes all online users and send to client */
void broadcastCurrentUsersToUser(client_t *client) {
	char buf[BUF_LEN + 1];
	memset(buf, 0, sizeof(buf));
	strcat(buf, "List of users online: \r\n");
	for (int i = 0; i < MAX_CLIENTS; i++) {
		if (clients[i]) {
			strcat(buf, clients[i]->client_username);
			strcat(buf, "\r\n");
		}
	}
	if (write(client->client_sockfd, buf, strlen(buf)) < 0) {
		perror("Error: Failed to write to client");
	}

}

/* print to stdout of server client connects and disconnects, also broadcast to clients */
void logAndBroadcastJoinOrLeave(client_t *client, int err_code) {

	char buf[BUF_LEN + 1];
	memset(buf, 0, sizeof(buf));
	strcat(buf, client->client_username);
	switch(err_code) {
		case 1:
			strcat(buf, " has joined the chat server.\r\n\0");
			break;
		case 2:
			strcat(buf, " has left the chat server.\r\n\0");
			break;
	}
	for (int i = 0; i < MAX_CLIENTS; i++) {
		if (clients[i]) {
			if (write(clients[i]->client_sockfd, buf, strlen(buf)) < 0) { /* send message to client */
				perror("Error: Failed to write to client");
			}
		}
	}
	printf(buf);

}

/* consumes the message queue whenever condition signaled */

/* handles the client, provides initialization, reads, writes, notifications */
void *clientCloneFactory(void *args) {
	int *client_sockfd = args;
	char buf[BUF_LEN + 1];

	/* create client and get client's name and fd */
	char buf_init[BUF_LEN + 1];
	if (read(*client_sockfd, buf_init, BUF_LEN) < 0){
		perror("Error: Failed to read from socket");
	}
	client_t *client = (client_t*) malloc(sizeof(client_t));
	client->client_sockfd = *client_sockfd;
	strcpy(client->client_username, buf_init); /* client sends us username, copy to buffer in client data structure */

	/* add client to queue */
	clientAddQueue(client); /* has locks in it */

	/* increment concurrent connections */

	connected++;

	/* send welcome msg to client, broadcast client's username, 
	and tell who else is currently in the chat server */
	logAndBroadcastJoinOrLeave(client, ERR_JOIN);
	if (write(client->client_sockfd, WELCOME_MSG, strlen(WELCOME_MSG)) < 0) {
		perror("Error: Failed to write to client");
	}
	broadcastCurrentUsersToUser(client);

	/* block until client message received
	if a client presses ^C, that gets written in fd buffer */ 
	while (read(client->client_sockfd, buf, BUF_LEN) > 0) {
		if (strstr(buf, ":exit\n") != NULL) {
			break;
		}
		messageAddQueue(buf);
		// pthread_cond_signal(&cond); /* tell the consumer thread to consume the message queue */ // flushMessageQueue();
		flushMessageQueue();
	}

	/* close client socket, broadcast to all users client disconnected */
	if (close(client->client_sockfd) < 0 ) {
		perror("Error: Failed to close socket");
	}
	clientDeleteQueue(client);
	logAndBroadcastJoinOrLeave(client, ERR_LEAVE); /* broadcast after removing client from queue, otherwise bad fd (since fd closed) */

	/* free up memory, yield thread */
	free(client);
	connected--;
	pthread_detach(pthread_self());
}

int main(int argc, char *argv[]){
	/* initialize setting variables */
	char *username = USERNAME;
	int port = PORT_NO;
	int sockfd;
	int client_sockfd;
	struct sockaddr_in server_addr;
	pthread_t tid;
	// pthread_t tid2;

	/* parse arguments*/
	parseArgs(argv, &username, &port); 

	/* socket settings */
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	/* open socket, bind, and listen to it*/
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Error: Failed to open socket");
		exit(1);
	}
	if (bind(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
		perror("Error: Failed to bind to socket");
		exit(1);
	}
	if (listen(sockfd, 5) < 0) {
		perror("Error: Socket listening failed");
		exit(1);
	}

	/* display server has started */
	puts(SERVER_START);

	/* launch consumer thread */
	// pthread_create(&tid2, NULL, consumerThread, NULL);

	while (1) {
		/* accept client connection */
		if ((client_sockfd = accept(sockfd, NULL, NULL)) < 0) {
			perror("Error: Failed to accept the incoming connection");
			continue;
		}

		/* reject client immediately if max clients reached */
		if (connected >= MAX_CLIENTS) {
			if (write(client_sockfd, SERVER_FULL, strlen(SERVER_FULL)) < 0) {
				perror("Error: Failed to write to client");
			}
			close(client_sockfd);
			continue;
		}

		/* create pthread */
		pthread_create(&tid, NULL, clientCloneFactory, &client_sockfd);
	}
	close(sockfd);
}