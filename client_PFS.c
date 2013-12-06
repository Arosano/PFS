#include "client_PFS.h"

char file_list[2056];
int client_port;\
char client_id;

int main(int argc, char *argv[]){

	client_id = *argv[1];
	char* server_ip = argv[2];
	char input[64];
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
	pthread_create(&get_thread, &attr, handle_get, (void *) NULL);
	/*setup local address information*/

	
	/*set up server address information*/
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = inet_addr(server);
	listen_addr.sin_port = htons(server_port);

	connect(sd, (struct sockaddr *) server_addr, sizeof(server_addr));
	send_buffer = client_id;

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

	while(strncmp(input, "exit", 4) != 0 ){

		Printf("Input a command:\nls - request master file list from server\n
			get <id> <Filename> - request file with < filename > directly from
			 peer with ID < id >\n
			 exit - close running client\n");
		gets(input);

		if(strncmp(input, "ls", 2) == 0){
			send(sd, input, strlen(input), 0);
			recv(sd, file_list, 2056, 0);
			printf("file list received: %s\n", file_list);

		}
		else if(strncmp(input, "get", 3) == 0){


		}
	}


	


}



void build_file_list(){

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
	    fp = fopen (ent->d_name, r+);
	    fseek(fp, 0L, SEEK_END);
		sz = ftell(fp);
		sprintf(&file_list[bytes_stored],"%d || %c :: ", sz, client_id);

		itoa(sz, int_hold, 10);

		bytes_stored += strlen(int_hold);

		fclose(fp);

	  }

	  closedir (dir);

	} else {
	  /* could not open directory */
	  perror ("");
	  return NULL;
	}
}

void* handle_get(){

	int opt = 1;
	int sd, client_sd;
	int recv_buffer[3];
	struct sockaddr_in listen_addr;
	struct sockaddr_in *sin_store;
	struct sockaddr store_addr;
	socklen_t store_addrlen;
	socklen_t addr_len;
	addr_len = sizeof(listen_addr);

	if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
			perror("socket");
      		return NULL;
	}

	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = INADDR_ANY;
	listen_addr.sin_port = 0;

	if(setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == -1){
			perror("set sock opt error");
			return NULL; 
	}

	if(bind(sd, (struct sockaddr *) &listen_addr, addr_len) < 0){
		perror("bind");
		return NULL;
	}

	getsockname(sd, store_addr, store_addrlen);
	sin_store = (struct sockaddr_in *) store_addr;
	client_port = (int) ntohs(sin_store -> sin_port);

	printf("client_port = %d\n", client_port);

	while(1){

		if(listen(sd, 10) != 0){

			perror("listen");
			return NULL;
		}


		client_sd = accept(sd, (struct sockaddr*)&listen_addr, &addr_len);

		recv(client_sd, recv_buffer, 3, 0);
		if(strncmp(recv_buffer,"get", 3) == 0){

			send(client_sd, file_list, strlen(file_list), 0);
		}

		close(client_sd);



	}


}