/* Networks Program 3
 * Lauren Ferrara - lferrara
 * Charlie Newell - cnewell1
 *
 * Server side of FTP
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
#define MAX_PENDING 5

int listDirectory(char* buffer){
	int new_len;
	FILE *fp = popen("ls -l", "r");
	if(fp != NULL){
        	new_len = fread(buffer, sizeof(char), BUFSIZE, fp);
		if(ferror(fp) != 0){
            		fputs("error reading file" , stderr);
        	} else {
           		buffer[new_len++] = '\0';
        	}
        	pclose(fp);
    	}
	return new_len;
}

int main (int argc, char *argv[]) {

	//variable declaration
	int port, s, filelen, len,  s_new;
	int waiting = 1;
	struct sockaddr_in sin;	
	int opt = 1; /* 0 to disable options */
	char buffer[BUFSIZE], operation[BUFSIZE];;


	//check command line arguments
	if (argc != 2) {
		fprintf(stderr, "Usage: %s [Port]\n", argv[0]);
		exit(1);
	}
	port = atoi(argv[1]);

	//build address data structure
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);

	//setup socket
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0 ) {
		perror("FTP Server: Unable to open socket\n");
		exit(1);
	}

	//set socket option to allow reuse of port
	if ((setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(int))) < 0 ) {
		perror("FTP Server: Unable to set socket options\n");
		exit(1);
	}

	//bind socket
	if ((bind(s, (struct sockaddr*)&sin, sizeof(sin))) < 0 ) {
		perror("FTP Server: Unable to bind to socket\n");
		exit(1);
	}



	//listen for connection
	if ((listen(s, MAX_PENDING)) < 0 ) {
		perror("FTP Server: Unable to listen\n");
		exit(1);
	}

	while(waiting) {
		
		//accept connection
		if ((s_new = accept(s, (struct sockaddr*)&sin, &len)) < 0 ) {
			perror("FTP Server: Unable to accept connection\n");
			exit(1);
		}

		while(1) {
	
			bzero((char*)&buffer, sizeof(buffer));
			
			//receive message from client
			if ((len = recv(s_new, buffer, sizeof(buffer), 0)) == -1 ) {
				perror("FTP Server: Unable to receive\n");
				exit(1);
			}
			
			if (len == 0) break;
			
			if (!strcmp(buffer, "QUIT")) {
				printf("FTP Server: Client has closed connection.\n");
				break;
			} else if (!strcmp(buffer, "LIST")) {
				//Do list commands
				bzero(buffer, BUFSIZE);	
				int x = listDirectory(buffer);
	    			int32_t conv = htonl(x);
    				char *data = (char*)&conv;
    				int size_int = sizeof(conv);
				// Write size of listing as 32-bit int		
				if (write(s_new, data, size_int) < 0) {
					perror("FTP Server: Error sending size of directory listing\n");
					exit(1);
				}
				
				// Write listing
				if (write(s_new, buffer, BUFSIZE) < 0) {
					perror("FTP Server: Error listing directory contents\n");
					exit(1);
				}
			} else {
				// Get filename/directory name length from client
                                int16_t ret;
                                char *data = (char*)&ret;
                                int int_size = sizeof(ret);
                                if (recv(s_new, data, int_size, 0) < 0) {
					perror("FTP Server: Error receiving file/directory size\n");
					exit(1);
				}
                                int name_size = ntohs(ret);
				printf("filename size: %d\n", name_size);
				char filename[name_size + 1];
				bzero(filename, name_size + 1);
				// Get filename/directory name from client
				if (recv(s_new, filename, name_size, 0) == -1 ) {
                                        perror("FTP Server: Unable to receive filename\n");
                                        exit(1);
                                }
				filename[name_size] = '\0';

				// Begin other operations
				if (!strcmp(buffer, "DWLD")) {
					//Download file commands
				} else if (!strcmp(buffer, "UPLD")) {
					//Upload file commands
				} else if (!strcmp(buffer, "DELF")) {
					//Delete file commands
				} else if (!strcmp(buffer, "RDIR")) {
					//remove directory commands
				} else if (!strcmp(buffer, "CDIR")) {
					//change directory commands
				}
			}
				

		}
		close(s_new);

	}
	exit(0);
}

