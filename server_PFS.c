#include "server_PFS.h"

fd_set connected_clients;//list of connected clients sds
int listen_sd; //socket descriptor for listening on
int max_sd;//max sd number
int connections[MAX_CONNECTIONS];//array of sds containing currently connected client sds
char connection_id[MAX_CONNECTIONS];//array of id's corresponding to connected clients
int total_connections;//total current clients connected
int file_bytecount;//total bytes stored in master file list
char file_list[1024*MAX_CONNECTIONS];//master file list
char client_info[MAX_CONNECTIONS][2048];//store client info:ID, IP, Port



/* Files received as: Source ID,Source IP,Source port, file count
   Then: File Name, File Size*/

int main(int argc, char *argv[]){}
	bzero(file_list, sizeof(file_list));
	int opt = 1;
	file_bytecount = 0;
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




		if(sockets_read < 0){
			perror("select");
			exit(1);
		}
		else if(sockets_read == 0){
			//No sockets are ready for reading from
			printf("Waiting for client requests...\n");
			fflush(stdout);
		}
		else{//there are sockets to be read from, handle the new connections and then deal with
			//incoming data accordingly per socket

			if(FD_ISSET(listen_sock, &connected_clients))
				handle_new_connection();

			for(i = 0; i < MAX_CONNECTIONS; i++){
				if(FD_ISSET(connections[i], &connected_clients))
					handle_data(i);
			}





		}









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
	int i,j;	     //to keep track of current socket in list
	int inc_conn; /* sd for incoming connection */
	char inc_buf[1024];/*buffer to accept requesting sockets ID*/
	char send_buf[1024];
	char rec_id; //id received from recv
	int rec_port;
	struct sockaddr_in inc_addr;
	socklen_t inc_addrlen;
	/*incoming connection is request, check to make sure no client has the name ID.
		If ID is not in use, continue, else remove the client sd from the list*/

	inc_conn = accept(listen_sock, NULL, NULL);
	
	if (inc_conn < 0) {

		perror("accept");
		exit(EXIT_FAILURE);
	}

	setnonblocking(inc_conn);

	for (i = 0; i < MAX_CONNECTIONS; i ++){
		//loop through all current connections 
		if(connection[i] == 0) {//if connection is valid

			recv(connection[i], inc_buf, 1, NULL);//receive id on socket
			rec_id = inc_buf[0];

			if(strrchr(connection_id, rec_id)){//check to see if the ID is already in 
				//the set of connections

				sprintf(send_buf, "Error: connection with ID {%c} already in session, closing connection", rec_id);
				printf("\nError: connection with ID {%c} already in session, closing connection
							\n", rec_id);
				//might not be necessary
				send(inc_conn,send_buf, strlen(send_buf), NULL);//send over error message

				close(inc_conn);//close the incoming connection and don't add it to the list
				break;

			}
			else{//if not add the connection to the list of current connections

				bzero(inc_buf, sizeof(inc_buf));
				connection_id[i] = rec_id;

				printf("\nConnection accepted:   FD=%d; Slot=%d; ID:%c\n",
					inc_conn, i, connection_id[i]);
				sprintf(send_buf,"connection accepted");

				send(inc_conn,send_buf, strlen(send_buf), NULL);//send over connected message
				
				bzero(send_buf, sizeof(send_buf));

				recv(connection[i], inc_buf, 10, NULL);//receive port on socket

				rec_port = inc_buf;

				bzero(inc_buf, sizeof(inc_buf));
				connections[i] = inc_conn;
				total_connections += 1;
				/* Files stored as: Source IP, Source port: Source ID, file count File Name,
					 File Size*/
   				

   				/*Maybe file count isn't necessary, come up with a good way to organize files
   				by owner so that clients can easily read the master file list and request
   				files from each other*/
				recv(connection[i], inc_buf, 1024, NULL);
				getpeername(connection[i], (struct sockaddr *) &inc_addr, &inc_addrlen);
				
				sprintf(client_info[i],"%s, %d, %c: %s", inet_ntoa(inc_addr.sin_addr), 
					rec_port, rec_id, inc_buf);
				masterfl_insert(i);
				
				
	
				
				for(j = 0; j < MAX_CONNECTIONS; j++){
					/*broadcast updated master file list to all connected clients*/
					if((connection[j] != connection[i]) && (connection[j] != 0) &&
										 FD_ISSET(connection[j], &connected_clients)){

						send(connection[j], file_list, file_bytecount, NULL);


					}

				}
					

				

				break; //leave for loop
			}
			
		}

	}
	
}

void handle_data(int i){

	char rec_buffer[24];
	char send_buffer[1024];
	/*Handle master file list request by clients*/
	if(recv(connections[i], rec_buffer, 6) < 0){

		printf("connection to client {%c} lost\n", connection_id[i]);
		close(connections[i]);
		connections[i] =0;
		masterfl_remove(i);

	}
	if(strncmp(rec_buffer, "ls", 2) == 0){

		send(connection[i], file_list, file_bytecount, NULL);
	}
	if(strncmp(rec_buffer, "exit", 4) == 0){
		printf("connection to client {%c} closed\n", connection_id[i]);
		close(connections[i]);
		connections[i] = 0;
		masterfl_remove(i);
	}



}

void masterfl_insert(int i){
	
	strncpy(&file_list[file_bytecount], client_info[i], strlen(client_info[i]));

	file_bytecount += strlen(client_info[i]);
		
}

void masterfl_remove(int i){

	int bytecount = 0;
	int remain = 0;
	int j;
	if(i < total_connections - 1){

		
		//get bytecount up to position to be removed
		for(j = 0; j < i; j++){

			bytecount = strlen(client_info[j]);
		}

		remain = file_bytecount - bytecount;
		bzero(file_list[bytecount], remain);
		remain -= strlen(client_info[i]);
		for(j = i + 1; j < total_connections; j++){

			strncpy(&file_list[bytecount], client_info[j], strlen(client_info[j]));
			bytecount += strlen(client_info[j]);
		}

		file_bytecount = bytecount;

	}
	else if(i == total_connections - 1){
		bzero(file_list[file_bytecount - strlen(client_info[i])],client_info[i]);
	}

}

