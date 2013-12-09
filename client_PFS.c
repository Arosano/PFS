#include "client_PFS.h"

char file_list[5086];
int client_port;
char client_id;
int sd;
int total_bytes;


int main(int argc, char *argv[]){

	client_id = *argv[1];
	char* server_ip = argv[2];
	char input[128];
	int set = 0;
	char recv_buffer[256];
	char send_buffer[512];
	int server_port = atoi(argv[3]);
	struct sockaddr_in server_addr;
	

	if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("socket");
		exit(1);
	}
	

	build_file_list();

	pthread_t get_thread;
	pthread_t get_command_thread;
	pthread_t update_thread;
	pthread_create(&get_thread, &attr, handle_inc_get, (void *) NULL);
	sleep(1);
	/*setup local address information*/

	
	/*set up server address information*/
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(server_ip);
	server_addr.sin_port = htons(server_port);
	bzero(send_buffer, sizeof(send_buffer));
	connect(sd, (struct sockaddr *) &server_addr, sizeof(server_addr));
	send_buffer[0] = client_id;

	send(sd, send_buffer, strlen(send_buffer), 0);

	recv(sd, recv_buffer, 20, 0);

	if(strncmp(recv_buffer, "Error", 5) == 0){
		printf("%s\n", recv_buffer);
		close(sd);
		exit(1);
	}
	else
		printf("\n%s\n\n", recv_buffer);


	bzero(send_buffer, sizeof(send_buffer));
	bzero(recv_buffer, sizeof(recv_buffer));
	/*send over the clients port*/
	sprintf(send_buffer, "%d", client_port);
	
	send(sd, send_buffer,5, 0);


	bzero(send_buffer, sizeof(send_buffer));
	/*deal with user commands*/
	/*send file list*/
	send(sd, file_list, total_bytes, 0);
	pthread_create(&update_thread, &attr, update_recv, (void *) &sd);
	while(strncmp(input, "exit", 4) != 0 ){
		setnonblocking(sd);
		
		if(set == 0){
			printf("\n\nInput a command:\n\n    ls - request master file list from server\n"
				"\n    get <id> <Filename> <filesize> <ip> <port> - get file from another client\n"
				 "\n    exit - close running client\n\n");
			gets(input);

			
			if(strncmp(input, "ls", 2) == 0){
				send(sd, input, 2, 0);
				set = 1;
				

			}
			
			if(strncmp(input, "get", 3) == 0){

				handle_get_command(input);


			}
			
			bzero(recv_buffer, sizeof(recv_buffer));
		}
		else{
			sleep(2);
			set = 0;
		}
	}

	send(sd, input, 4, 0);
	exit(0);


	


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

	    /*open file, get size, store size in file list*/
	    
	    if(strncmp(ent->d_name, "client", 6)){
	    	
		    fp = fopen (ent->d_name, "r+");
		    

		    if(fp){
		    	
		    	bzero(int_hold, 6);
	   	 		sprintf (&file_list[bytes_stored],"%s || ", ent->d_name);
	    		bytes_stored += strlen(ent->d_name) + 4;
			    fseek(fp, 0L, SEEK_END);
				sz = ftell(fp);
				sprintf(&file_list[bytes_stored],"%d || %c :: ", sz, client_id);

				sprintf(int_hold, "%d", sz);

				bytes_stored += strlen(int_hold) + 9;
				

				fclose(fp);
			}
		}

	  }
	  total_bytes = bytes_stored;

	  closedir (dir);

	} else {
	  /* could not open directory */
	  perror ("");
	  exit(1);
	}
}

void* update_recv(void* sock){
	
	char recv_buffer[1024];
	while(1){

		
		if(recv(sd, recv_buffer, sizeof(recv_buffer), 0) > 0){
				printf("\n\nFile List Update Received: \n\n%s\n", recv_buffer);
		}
		

		fflush(stdout);
		bzero(recv_buffer, sizeof(recv_buffer));
	}

}

void* handle_inc_get(){


	char filename[16];
	int filesize;
	FILE* file;
	int opt = 1;
	int sd, client_sd;
	char recv_buffer[32];
	char send_buffer[2048];
	struct sockaddr_in listen_addr;
	struct sockaddr_in sin_store;
	
	int retval_pass = 0;
	int retval_fail = 1;
	socklen_t store_addrlen;
	socklen_t addr_len;
	addr_len = sizeof(listen_addr);
	store_addrlen = sizeof(sin_store);

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

	if(getsockname(sd, (struct sockaddr *) &sin_store, &store_addrlen) != 0){
		perror("getsockname:");
		pthread_exit(&retval_fail);

	}
	
	client_port = (int) ntohs(sin_store.sin_port);

	


	while(1){

		if(listen(sd, 10) != 0){

			perror("listen");
			pthread_exit(&retval_fail);
		}


		client_sd = accept(sd, (struct sockaddr*)&listen_addr, &addr_len);

		while(recv(client_sd, recv_buffer, 64, 0) < 0);
		if(strncmp(recv_buffer,"get", 3) == 0){

			sscanf(recv_buffer,"%*s %s %d",filename, &filesize);
			
			file = fopen(filename, "r+");
			fread(send_buffer, 1, filesize, file);	
			send(client_sd, send_buffer, filesize, 0);

			
		}
		fclose(file);
		close(client_sd);
		bzero(send_buffer, sizeof(send_buffer));
		bzero(recv_buffer, sizeof(recv_buffer));


	}


}

void handle_get_command(char* data){
	
	
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

	

	if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
			perror("socket");
      		return;
	}

	if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == -1){
			perror("set sock opt error");
			return;
	}

	sscanf(data, "%*s %*c %s %i %s %i", filename, &filesize, ip, &port);
	sprintf(get_request, "get %s %d", filename, filesize);
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

