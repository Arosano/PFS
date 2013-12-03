#include "server_PFS.h"

fd_set connected_clients;//list of connected clients sds
int listen_sd; //socket descriptor for listening on
int max_sd;//max sd number
int connections[MAX_CONNECTIONS];//array of sds containing currently connected client sds
char connection_id[MAX_CONNECTIONS];


int main(int argc, char *argv[]){}
	
	int opt = 1;
	int i;
	int sockets_read;
	
	int new_sd;//newly accepted socket descriptor
	struct sockaddr_in server_addr; //server address
	socklen_t addrlen;

	char buf[256];
	int nbytes;

	struct timeval timeout;

	if((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("server socket error");
		exit(1);
	}//create client socket

	if(setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == -1){
			perror("set sock opt error");
			exit(1);
	}//set socket options such that port address can be reused
	set_nonblocking(listen_sock);
	
	memset((char *) &server_address, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);


	FD_SET(listen_sock, &connected_clients);

	if(bind(listen_sock, (struct sockaddr *) &server_addr, sizeof(server_address)) < 0 ) {

		perror("bind");
		close(listen_sock);
		exit(1);

	}

	listen(listen_sock, MAX_CONNECTIONS);


	max_sd = listen_sock;//set max sd to the current listening sd
	bzero(connections, sizeof(connections);

	while(1){

		build_select_list();
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		sockets_read = select(max_sd+1, &connected_clients, (fd_set *) 0, (fd_set *)0,
								 &timeout);




		








	}

}

void setnonblocking(sock){

	int opts;

	opts = fcntl(sock,F_GETFL);//get file access mode and file status flags

	if (opts < 0) {
		perror("fcntl(F_GETFL)");
		exit(EXIT_FAILURE);
	}

	opts = (opts | O_NONBLOCK);//set opts to non blocking

	if (fcntl(sock,F_SETFL,opts) < 0) {//set socket to non blocking
		perror("fcntl(F_SETFL)");
		exit(EXIT_FAILURE);
	}

	return;
}

void build_select_list() {

	int i;//to keep track of current socket in list	     
	
	FD_ZERO(&connected_clients);//zero out fd_set connected clients
	
	
	
	FD_SET(sock,&socks);
	
	/* Loop through all connections and add
		the corresponding sd to the fd_set */
	
	for (i= 0; i < MAX_CONNECTIONS; i++) {

		if (connectlist[i] != 0) {

			FD_SET(connectlist[i], &connected_clients);

			if (connectlist[i] > max_sd)
				max_sd = connectlist[i];
		}
	}
}

void handle_new_connection() {
	int i;	     //to keep track of current socket in list
	int inc_conn; /* sd for incoming connection */
	char inc_buf[1];/*buffer to accept requesting sockets ID*/
	char rec_id; //id received from recv
	/*incoming connection is request, check to make sure no client has the name ID.
		If ID is not in use, continue, else remove the client sd from the list*/

	inc_conn = accept(listen_sock, NULL, NULL);

	if (inc_conn < 0) {

		perror("accept");
		exit(EXIT_FAILURE);
	}

	setnonblocking(inc_conn);

	for (i = 0; (i < MAX_CONNECTIONS); i ++){

		if (connected_clients[i] == 0) {

			recv(connected_clients[i], inc_buf, 1, NULL);
			rec_id = inc_buf[0];
			if(strrchr(connection_id, rec_id)){
				printf("Error: connection with ID {%c} already in session\n", rec_id);


			}
			else{
				printf("\nConnection accepted:   FD=%d; Slot=%d; ID:%c\n",
					inc_conn, i, connection_id[i]);
				connected_clients[i] = inc_conn;
				break;
			}
			
		}

	}
	
}

