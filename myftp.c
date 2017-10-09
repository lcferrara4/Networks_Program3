/* Networks Program 3
 * Lauren Ferrara - lferrara
 * Charlie Newell -
 *
 * Client side of FTP
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUFSIZE 4096

int main (int argc, char *argv[]) {

	// Variable declaration
	int port, s, size;
	char *server_name;
	struct hostent *server;
	struct sockaddr_in server_addr;
	char buffer[BUFSIZE];
	int running = 1;
	int opt = 1; /* 0 to disable options */
	char filename[BUFSIZE];

	// Check command line arguments
	if (argc != 3) {
		fprintf(stderr, "Usage: %s Server_Name [Port]\n", argv[0]);
		exit(1);
	}
	server_name = argv[1];
	port = atoi(argv[2]);

	// Translate server name to IP address
	server = gethostbyname(server_name);
	if (!server) {
		fprintf(stderr, "FTP Client: Unknown server %s\n", server_name);
		exit(1);
	}

	// Open socket
	if ((s = socket (PF_INET, SOCK_STREAM, 0)) < 0 ) {
		perror("FTP Client: Unable to open socket\n");
		exit(1);
	}

	// Load buffer
	bzero(buffer, BUFSIZE);

	// Build server address data structure
	bzero((char*) &server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	bcopy((char*) server->h_addr, (char *) &server_addr.sin_addr.s_addr, server->h_length);
	server_addr.sin_port = htons(port);

	setsockopt(s, SOL_SOCKET, SO_REUSEADDR,(char *)&opt, sizeof(int));

	// Connect to socket
	if (connect(s, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0 ) {
		perror("FTP Client: Unable to connect to socket\n");
		exit(1);
	}

	// Prompt user for operation
	while (running) {
		printf("Enter FTP operation: ");
		scanf("%s", buffer);
		size = strlen(buffer);	
		if (write(s, buffer, size) < 0) {
			perror("FTP Client: Error writing to socket\n");
			exit(1);	
		}

		// Send message to server
		if (!strcmp(buffer, "QUIT")) {
			// Exit client connection
			printf("FTP Client: Connection has been closed.");
			running = 0;
			break;
		} else if (!strcmp(buffer, "LIST")) {
			// List directory at the server
	
		} else {
			// Get file or directory name for operation
			printf("Enter file or directory name: ");
			scanf("%s", filename);		
			size = strlen(filename);	
                	if (write(s, filename, size) < 0) {
                        	perror("FTP Client: Error writing to socket\n");
                        	exit(1);
                	}
			if (!strcmp(buffer, "DWLD")) {
				// Download file
			} else if (!strcmp(buffer, "UPLD")) {
				// Upload file
			} else if (!strcmp(buffer, "DELF")) {
				// Delete file
			} else if (!strcmp(buffer, "MDIR")) {
				// Make directory
			} else if (!strcmp(buffer, "RDIR")) {
				// Remove directory
			} else if (!strcmp(buffer, "CDIR")) {
				// Change to a different directory
			}
		}

		
	}
	exit(0);
}


