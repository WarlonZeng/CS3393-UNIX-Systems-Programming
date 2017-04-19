#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h> 
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUF_LEN 1024
#define PORT_NO 20000
#define USERNAME "CLIENT"

int parse_args(char* argv[], char** username, char** ip_addr, int* port);
void header_wrap(char* buf, char* name);

// parse arguments with a switch-case.. much more organized
int parse_args(char* argv[], char** username, char** ip_addr, int* port) {
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
				case 'i': 
					*ip_addr = argv[i+1];
					break;
			}
		}
	}
	return 0;
}

// move block of memory of msg to the right, write name on the left, finish in middle with :
void header_wrap(char* buf, char* name) {
    size_t len = strlen(name);
    size_t i;

    memmove(buf + len + 1, buf, strlen(buf) + 1);
    for (i = 0; i < len; ++i) {
        buf[i] = name[i];
    }
	buf[len] = ':';
}

int main(int argc, char* argv[]){
	char* username;
	char* ip_addr;
	int port = PORT_NO;
	
	parse_args(argv, &username, &ip_addr, &port);

	// ------------------------------------------------------------------------

	int sockfd;
	struct sockaddr_in server_addr;
	struct hostent *server;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Error: Failed to open socket");
	}
	server = gethostbyname(ip_addr);
	if ((server = gethostbyname(ip_addr)) == NULL) {
		fprintf(stderr,"Error: Failed to specify host\n");
		exit(1);
	}

    server_addr.sin_addr = *(struct in_addr *)*server -> h_addr_list;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

	if (connect(sockfd,(struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		perror("Error: Failed to connect to server");
		exit(1);
	}

	// ------------------------------------------------------------------------

	fd_set readfds;
	fd_set testfds;

	FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    FD_SET(0, &readfds); 

    // ------------------------------------------------------------------------

	while (1) {
		testfds = readfds;
        select(FD_SETSIZE, &testfds, NULL, NULL, NULL); // block until something happens in one of the fds in the FD_SET

		for (int fd = 0; fd < FD_SETSIZE; fd++) { //  check out the fd in FD_ISSET... 0 = stdin, sockfd = server
			if (FD_ISSET(fd, &testfds)) {

				if (fd) {
					char buffer[BUF_LEN + 1];
					memset(buffer, 0, sizeof(buffer));
					if (read(fd, buffer, BUF_LEN) < 0) {
						perror("Error: Failed to read from server"); // implies server might be dead or terminating us
						FD_CLR(fd, &readfds);
						close(fd);
					}
					printf("%s", buffer);
				}

				else if (fd == 0) { // 
					char buffer[BUF_LEN + 1];
					memset(buffer, 0, sizeof(buffer));
					if (fgets(buffer, BUF_LEN, stdin) == NULL) {
						perror("Error: Failed to read stdin");
						exit(1);
					}
					header_wrap(buffer, username);
					if (write(sockfd, buffer, strlen(buffer)) < 0) {
						perror("Error: Failed to write to client"); 
					}
				}
			}
		}
	}
	close(sockfd);
}