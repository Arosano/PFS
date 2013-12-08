#include "client_PFS.h"

char file_list[1024];
int client_port;
char client_id;

int main(int argc, char *argv[]){

	client_id = *argv[1];
	char* server_ip = argv[2];
	char input[128];
	char recv_buffer[256];
	char send_buffer[512];
	int server_port = atoi(argv[3]);
	struct sockaddr_in server_addr;
	int sd;

	if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("socket");
		exit(1);
	}

	build_file_list();

	pthread_t get_thread;
	pthread_t get_command_thread;
	pthread_create(&get_thread, &attr, handle_get, (void *) NULL);
	/*setup local address information*/

	
	/*set up server address information*/
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(server_ip);
	server_addr.sin_port = htons(server_port);

	connect(sd, (struct sockaddr *) &server_addr, sizeof(server_addr));
	send_buffer[0] = client_id;

	send(sd, send_buffer, strlen(send_buffer), 0);

	recv(sd, recv_buffer, 256, 0);

	if(strncmp(recv_buffer, "Error", 5) == 0){
		printf("%s\n", recv_buffer);
		close(sd);
		exit(1);
	}
	else
		printf("%s\n", recv_buffer);


	bzero(send_buffer, sizeof(send_buffer));
	bzero(recv_buffer, sizeof(recv_buffer));
	/*send over the clients port*/
	sprintf(send_buffer, "%d", client_port);
	send(sd, send_buffer, strlen(send_buffer), 0);

	bzero(send_buffer, sizeof(send_buffer));
	/*deal with user commands*/
	while(strncmp(input, "exit", 4) != 0 ){
		setnonblocking(sd);
		if(recv(sd, recv_buffer, sizeof(recv_buffer), 0) > 0){
			printf("File List Update Received: %s\n", recv_buffer);
		}

		printf("\nInput a command:\nls - request master file list from server\n"
			"get <id> <Filename> <filesize> <ip> <port> - request file with  <filename> directly from"
			 "peer with ID <id>, ip address <ip> and port number <port>\n"
			 "exit - close running client\n");
		gets(input);

		if(strncmp(input, "ls", 2) == 0){
			send(sd, input, strlen(input), 0);
			recv(sd, file_list, 2056, 0);
			printf("file list received: %s\n", file_list);

		}
		else if(strncmp(input, "get", 3) == 0){

			pthread_create(&get_command_thread, &attr, handle_get_command, input);


		}
		bzero(recv_buffer, sizeof(recv_buffer));
	}


	


}



void build_file_list(){

	int sz;
	char dir_namebuf[64];
	int bytes_stored = 0;
	FILE* fp;
	char int_hold[6];
	/*get the current directory*/
	getcwd(dir_namebuf, sizeof(dir_namebuf));


	DIR *dir;
	struct dirent *ent;
	/*open directory*/
	if ((dir = opendir (dir_namebuf)) != NULL) {

	  /* store all files in directory in file_list */
	  while ((ent = readdir (dir)) != NULL) {
	  	
	  	bzero(int_hold, 6);
	    sprintf (&file_list[bytes_stored],"%s || ", ent->d_name);
	    bytes_stored += strlen(ent->d_name);
	    /*open file, get size, store size in file list*/
	    printf("%s\n",ent->d_name);
	    fp = fopen (ent->d_name, "r+");
	    if(fp){
		    fseek(fp, 0L, SEEK_END);
			sz = ftell(fp);
			sprintf(&file_list[bytes_stored],"%d || %c :: ", sz, client_id);

			sprintf(int_hold, "%d", sz);

			bytes_stored += strlen(int_hold);

			fclose(fp);
		}

	  }

	  closedir (dir);

	} else {
	  /* could not open directory */
	  perror ("");
	  exit(1);
	}
}

void* handle_get(){

	int opt = 1;
	int sd, client_sd;
	char recv_buffer[3];
	struct sockaddr_in listen_addr;
	struct sockaddr_in *sin_store;
	struct sockaddr *store_addr;
	int retval_pass = 0;
	int retval_fail = 1;
	socklen_t store_addrlen;
	socklen_t addr_len;
	addr_len = sizeof(listen_addr);

	if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
			perror("socket");
      		pthread_exit(&retval_fail);
	}

	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = INADDR_ANY;
	listen_addr.sin_port = 0;

	if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == -1){
			perror("set sock opt error");
			pthread_exit(&retval_fail);
	}

	if(bind(sd, (struct sockaddr *) &listen_addr, addr_len) < 0){
		perror("bind");
		pthread_exit(&retval_fail);
	}

	getsockname(sd, store_addr, &store_addrlen);
	sin_store = (struct sockaddr_in *) store_addr;
	client_port = (int) ntohs(sin_store -> sin_port);

	printf("client_port = %d\n", client_port);

	while(1){

		if(listen(sd, 10) != 0){

			perror("listen");
			pthread_exit(&retval_fail);
		}


		client_sd = accept(sd, (struct sockaddr*)&listen_addr, &addr_len);

		recv(client_sd, recv_buffer, 3, 0);
		if(strncmp(recv_buffer,"get", 3) == 0){

			
		}

		close(client_sd);



	}


}

void* handle_get_command(void* data){
	pthread_detach(pthread_self());
	char* get_data = data;
	char ip[11];
	int port;
	int opt = 1;
	char filename[24];
	FILE* new_file;
	int filesize;
	struct sockaddr_in peer_addr;
	int sd;
	int retval_pass = 0;
	int retval_fail = 1;

	char get_request[32];
	char* recv_file_buffer;

	sprintf(get_request, "get %s", filename);

	if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
			perror("socket");
      		return NULL;
	}

	if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == -1){
			perror("set sock opt error");
			return NULL; 
	}

	sscanf(get_data, "%*s <%*c> <%s> <%i> <%s> <%i>", filename, &filesize, ip, &port);
	recv_file_buffer = malloc(sizeof(char) * filesize);

	new_file = fopen(filename, "wb+");

	peer_addr.sin_family = AF_INET;
	peer_addr.sin_addr.s_addr = inet_addr(ip);
	peer_addr.sin_port = htons(port);

	connect(sd, (struct sockaddr*) & peer_addr, sizeof(peer_addr));

	send(sd, get_request, strlen(get_request), 0);

	recv(sd, recv_file_buffer, filesize, 0);

	fwrite(recv_file_buffer, sizeof(char), filesize, new_file);

	fclose(new_file);
	close(sd);

	pthread_exit(&retval_pass);



}

void setnonblocking(int sock){

	int opts;

	opts = fcntl(sock, F_GETFL);//get file access mode and file status flags

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