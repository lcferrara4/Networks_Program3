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
int receiveInt(int bits, int socket) {
        int size = 0;
        if (bits == 32) {
                int32_t ret;
                char *data = (char*)&ret;
                int int_size = sizeof(ret);
                if (recv(socket, data, int_size, 0) < 0) {
                        perror("FTP Server: Error receiving file/directory size\n");
                        exit(1);
                }
                size = ntohl(ret);
        } else if (bits == 16) {
                int16_t ret;
                char *data = (char*)&ret;
                int int_size = sizeof(ret);
                if (recv(socket, data, int_size, 0) < 0) {
                        perror("FTP Server: Error receiving file/directory size\n");
                        exit(1);
                }
                size = ntohs(ret);
        }
	printf("%i\n", size);
        return size;
}

void sendInt(int x, int bits, int socket) {
        if (bits == 32) {
                int32_t conv = htonl(x);
                char *data = (char*)&conv;
                int size_int = sizeof(conv);
                if (write(socket, data, size_int) < 0) {
                        perror("FTP Server: Error sending size of directory listing\n");
                        exit(1);
                }

        } else if (bits == 16) {
                int16_t conv = htons(x);
                char *data = (char*)&conv;
                int size_int = sizeof(conv);
                if (write(socket, data, size_int) < 0) {
                        perror("FTP Server: Error sending size of directory listing\n");
                        exit(1);
                }

        }
}

void sendFileDir(int socket, char *filename) {
	// Get file or directory name for operation
	printf("Enter file or directory name: ");
	scanf("%s", filename);		
	int size = strlen(filename);	

	sendInt(size, 16, socket);
	// Send filename
	if (write(socket, filename, size) < 0) {
		perror("FTP Client: Error writing to socket\n");
		exit(1);
	}
}

int getFileSize(char* filename){
	int file_size;
	FILE *fp = fopen(filename, "r");
	fseek(fp, 0L, SEEK_END);
	file_size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	return file_size;
}

int main (int argc, char *argv[]) {

	// Variable declaration
	int port, s, size;
	char *server_name;
	struct hostent *server;
	struct sockaddr_in server_addr;
	char buffer[BUFSIZE];//, inner_buffer[BUFSIZE];
	int opt = 1; /* 0 to disable options */
	char filename[BUFSIZE];
	char operation[10];

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
	while (1) {

		printf("Enter FTP operation: ");
		bzero((char*)&operation, sizeof(operation));
		scanf("%s", operation);
		size = strlen(operation);	
		if (write(s, operation, size) < 0) {
			perror("FTP Client: Error writing to socket\n");
			exit(1);	
		}

		// Send message to server
		if (!strcmp(operation, "QUIT")) {
			// Exit client connection
			printf("FTP Client: Connection has been closed.\n");
			exit(0);
		} else if (!strcmp(operation, "LIST")) {
			// List directory at the server
			// Get directory listing size
			int listing_size = receiveInt(32, s);
			char listing[listing_size];
			//bzero((char*)&listing, sizeof(listing));
			if (read(s, listing, listing_size) < 0) {
				perror("FTP Client: Error reading from socket\n");
				exit(1);
			}
			printf("%s", listing);
		} else {
			bzero((char*)&filename, sizeof(filename));
			sendFileDir(s, filename);
			
			// Begin other operations
			if (!strcmp(operation, "DWLD")) {
				// Download file
				int file_size = receiveInt(32, s);
				int total_recv = 0;
				int i = 0;
				int x;
				char inner_buffer[BUFSIZE];

				printf("%i\n",file_size);

				if(file_size == -1) {
					printf("The desired file does not exist\n");
					continue;
				
				} else {
					FILE *fp = fopen(filename, "a");
					int recv_len = 0;
					int total = 0;
					
					while((recv_len = recv(s, inner_buffer, BUFSIZE, 0)) > 0) {
						total += recv_len;
						
						if(total > file_size) {
							inner_buffer[file_size - (total - recv_len)] = '\0';
							x = fwrite(inner_buffer, sizeof(char), file_size - (total - recv_len), fp);
							break;
						}
						x = fwrite(inner_buffer, sizeof(char), recv_len, fp);
						bzero(inner_buffer, BUFSIZE);
						if (total > file_size) break;
					}
					fclose(fp);
					
					
				}
			} else if (!strcmp(operation, "UPLD")) {
				// Upload file
				char inner_buffer[BUFSIZE];
				int x = getFileSize(filename);
				sendInt(x, 32, s);
				FILE *fp = fopen(filename, "r");		
				

				int sent = 0;
				int total = 0;
				while(sent = fread(inner_buffer, sizeof(char), BUFSIZE, fp)) {
					total += sent;
					if(write(s, inner_buffer, sent) < 0) {
						perror("FTP Server: Error sending file\n");
						exit(1);
					}
					bzero(inner_buffer, BUFSIZE);
					if(total >= x) break;
				}
				fclose(fp);


			} else if (!strcmp(operation, "DELF")) {
				// Delete file
				int confirm = receiveInt(32, s);
				if (confirm < 0) {
					printf("The file does not exist on server\n");
				} else {
					char inner_buffer[BUFSIZE];
					printf("Are you sure you want to delete? Yes or No: ");
					scanf("%s", inner_buffer);
					size = strlen(inner_buffer);
					if (write(s, inner_buffer, size) < 0) {
						perror("FTP Client: Error writing to socket\n");
						exit(1);
					}
					if (!strcmp(inner_buffer, "Yes")){
                                		confirm = receiveInt(32, s);
						if (confirm < 0) {
							perror("FTP Client: Error deleting the file\n");
							exit(1);
						} else {
							printf("File deleted successfully\n");
						}	
					} else {
 						printf("Delete abandoned by the user!\n");
					}
				}
			} else if (!strcmp(operation, "MDIR")) {
				// Make directory
				int confirm = receiveInt(32, s);
				if (confirm == -2) {
					printf("The directory already exists on server\n");
				} else if (confirm == -1) {
					printf("Error in making directory\n");
				} else {
					printf("The directory was successfully made\n");
				}
			} else if (!strcmp(operation, "RDIR")) {
				// Remove directory
				int confirm = receiveInt(32, s);
				if (confirm == -1) {
					printf("The directory doesn't exist on server\n");
				} else {
                                        char inner_buffer[BUFSIZE];
                                        printf("Are you sure you want to delete? Yes or No: ");
                                        scanf("%s", inner_buffer);
                                        size = strlen(inner_buffer);
                                        if (write(s, inner_buffer, size) < 0) {
                                                perror("FTP Client: Error writing to socket\n");
                                                exit(1);
                                        }
                                        if (!strcmp(inner_buffer, "Yes")){
                                                confirm = receiveInt(32, s);
                                                if (confirm < 0) {
                                                        perror("FTP Client: Error deleting the directory\n");
                                                        exit(1);
                                                } else {
                                                        printf("Directory deleted successfully\n");
                                                }
                                        } else {
                                                printf("Delete abandoned by the user!\n");
                                        }
				}
			} else if (!strcmp(operation, "CDIR")) {
				// Change to a different directory
				int confirm = receiveInt(32, s);
				if (confirm == -2) {
					printf("The directory doesn't exist on server\n");
				} else if (confirm == -1) {
					printf("Error in changing directory\n");
				} else {
					printf("Changed current directory\n");

				}
			}
		}
	}
	exit(0);
}

