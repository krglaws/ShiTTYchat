/*
 gcc -fopenmp -o ShiTTYchat ShiTTYchat.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <sys/socket.h>
#include <netinet/in.h>


void client(char * ip, int port);
void server(char * ip, int port);
int valid_args(int argc, char * argv[]);

#define MSGLEN 256

int main(int argc, char * argv[]){

	// check args
	if (!valid_args(argc, argv)) return 1;

	// grab args
	char c_or_s = argv[1][0];
	char * ip_addr = argv[2];
	int port = strtol(argv[3], NULL, 10);

	// check if current session is server or client
	c_or_s == 'c' ? client(ip_addr, port) : server(ip_addr, port);
}


int valid_args(int argc, char** argv){
	
	char usage[] = "Usage: ./ShiTTYchat <c or s> <ip> <port>\n";
	if (argc < 4){
		printf("Too few arguments supplied.\n%s", usage);
		return 0;
	}
	
	// is current session client or server?
	if (strlen(argv[1]) > 1 || (argv[1][0] != 'c' && argv[1][0] != 's')){
		printf("Unrecognized argument '%s'\n%s", argv[1], usage);
		return 0;
	}

	// check if ip is valid
	struct sockaddr_in sa;
	if (!inet_pton(AF_INET, argv[2], &(sa.sin_addr))){
		printf("Invalid IP: %s\n", argv[2]);
		return 0;
	}
	
	// make sure port number is valid
	int port = strtol(argv[3], NULL, 10);
	if (port < 1 || port > 65535){
		printf("Invalid port number %d.\n1 >= p <= 65535\n", port);
		return 0;
	}
	
	// ignore any extra args
	if (argc > 4) printf("Ignoring extra arguments...\n");

	return 1;

}// valid_args()


void client(char * ip, int port){
	
	// create socket
	int network_socket = socket(AF_INET, SOCK_STREAM, 0);
	
	// specify an address for the socket
	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(struct sockaddr_in));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
	server_address.sin_addr.s_addr = inet_addr(ip);

	// connect!
	int connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(struct sockaddr));

	// check for error with the connection
	if (connection_status) {
		printf("Failed to connect to server.\n");
		exit(connection_status);
	}
	printf("Connected to host %s...\n", ip);

	char msg_recv[MSGLEN];
	char msg_send[MSGLEN];

	// Start two threads: one for sending and one for receiving.
	// Each thread is assigned to a while loop. If either one breaks
	// from its loop, the program exits.
	# pragma omp parallel num_threads(2)
	{
		int id = omp_get_thread_num();

		if (id) {
	
			while(1) {

				if (recv(network_socket, msg_recv, MSGLEN, 0) <= 0) {
					printf("Connection lost.\n");
					break;
				}

				printf("%s", msg_recv);
			}
		}
		else {

			while(1) {

				fgets(msg_send, MSGLEN-1, stdin);

				if (msg_send[0] == '!') break;

				else if (send(network_socket, msg_send, MSGLEN, 0) < 0)
					printf("Failed to send\n");
			}
		}
		// if either thread reaches this point, exit
		printf("Exiting...\n");
		close(network_socket);
		exit(0);
	}

}// end client()


void server(char * ip, int port){
	
	// create server socket
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	
	// create client and server addresses
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
	memset(&server_address, 0, sizeof(struct sockaddr_in));
	memset(&client_address, 0, sizeof(struct sockaddr_in));
	
	// define server address
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
	server_address.sin_addr.s_addr = inet_addr(ip);

	// bind to specified IP and port
	if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) == -1){
		fprintf(stderr, "Failed to bind to port %d. Exiting...\n", port);
		close(server_socket);
		exit(-1);
	}

	printf("Awaiting connections...\n");
	
	// listen for connections
	if (listen(server_socket, 10) == -1){
		fprintf(stderr, "Listening Failure. Exiting...\n");
		close(server_socket);
		exit(-1);
	}

	int client_socket = accept(server_socket, NULL, NULL);

	printf("Connected to client...\n");

	char msg_recv[MSGLEN];
	char msg_send[MSGLEN];

	// Start two threads: one for sending and one for receiving.
	// Each thread is assigned to a while loop. If either one breaks
	// from its loop, the program exits.
	# pragma omp parallel num_threads(2)
	{
		int id = omp_get_thread_num();
		if (id){
			while (1){

				if (recv(client_socket, msg_recv, MSGLEN, 0) <= 0){
					printf("Connection lost.\n");					
					break;
				}
				printf("%s", msg_recv);
			}
		}
		else {	
			while (1){

				fgets(msg_send, MSGLEN-1, stdin);

				if (msg_send[0] == '!') break;

				else if (send(client_socket, msg_send, MSGLEN, 0) < 0)
					printf("Failed to send\n");
			}
		}

		// if either thread reaches this point, exit
		printf("Exiting...\n");
		close(server_socket);
		close(client_socket);
		exit(0);
	}

}// end server()

