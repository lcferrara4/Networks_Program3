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

#define BUFSIZE 4096
#define MAX_PENDING 5

int main (int argc, char *argv[]) {

	//variable declaration
	int port, s, filelen, len,  s_new;
	int waiting = 1;
	struct sockaddr_in sin;	
	int opt = 1; /* 0 to disable options */
	char buffer[BUFSIZE], filename[BUFSIZE];


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
		perror("simplex-talk:sockt");
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
			} else {
				if ((filelen = recv(s_new, filename, sizeof(filename), 0)) == -1 ) {
					perror("FTP Server: Unable to receive filename\n");
					exit(1);
				}
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
				

			//printf("Buffer: %s\n", buffer);
			//printf("Filename: %s\n", filename);
			bzero((char*)&buffer, sizeof(buffer));
			bzero((char*)&filename, sizeof(filename));
		}
		close(s_new);

	}
	exit(0);
}
