/* Networks Program 3
 * Lauren Ferrara - lferrara
 * Charlie Newell - cnewell1
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
#include <arpa/inet.h>

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
			printf("FTP Client: Connection has been closed.\n");
			running = 0;
			break;
		} else if (!strcmp(buffer, "LIST")) {
			// List directory at the server
			// Get directory listing size
			int32_t ret;
    			char *data = (char*)&ret;
    			int int_size = sizeof(ret);
			if (read(s, data, int_size) < 0) {
				perror("FTP Client: Error reading from socket\n");
				exit(1);
			}
			int listing_size = ntohl(ret);
			char listing[listing_size];
			if (read(s, listing, listing_size) < 0) {
				perror("FTP Client: Error reading from socket\n");
				exit(1);
			}
			printf("%s", listing);
		} else {
			// Get file or directory name for operation
			printf("Enter file or directory name: ");
			scanf("%s", filename);		
			size = strlen(filename);	

                        int16_t conv = htons(size);
                        char *data = (char*)&conv;
                        int size_int = sizeof(conv);
			// Send length of filename
			if (write(s, data, size_int) < 0) {
                                perror("FTP Client: Error writing to socket\n");
                                exit(1);
                        }
			// Send filename
                	if (write(s, filename, size) < 0) {
                        	perror("FTP Client: Error writing to socket\n");
                        	exit(1);
                	}

			// Begin other operations
			if (!strcmp(buffer, "DWLD")) {
				// Download file
				bzero(buffer, BUFSIZE);
				int32_t dwld_file_size;
				char *data = (char*)&dwld_file_size;
				int dwld_file_size_int = sizeof(dwld_file_size);
				if (read(s, data, dwld_file_size_int) < 0) {
					perror("FTP Client: Error reading dwld file size\n");
					exit(1);
				}

				int file_size = ntohl(dwld_file_size);
				int total_recv = 0;
				int i = 0;
				printf("%i\n",file_size);
				if(file_size == -1) {
					printf("The desired file does not exist\n");
					continue;
				} else {
					FILE *fp = fopen(filename, "a");
					while (total_recv < file_size) {
						if ((i = read(s, buffer, BUFSIZE)) < 0) {
							perror("FTP Client: Error dwld file\n");
							exit(1);
						}
						total_recv += i;
						if(total_recv > file_size) {
							buffer[file_size-(total_recv-i)] = '\0';
							fwrite(buffer, sizeof(char), file_size-(total_recv-i),fp);
							break;
						}
						fwrite(buffer, sizeof(char), i, fp);
					}
				}
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


