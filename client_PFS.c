#include "client_PFS.h"

char file_list[1024];

int main(int argc, char *argv[]){

	pthread_t get_thread;
	pthread_create(&get_thread, &attr, handle_get, (void *) NULL);




}

void* handle_get(){

	int opt = 1;
	int sd, client_sd;
	int recv_buffer[3];
	struct sockaddr_in listen_addr;
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
	while(1){

		if(listen(sd, 10) != 0){

			perror("listen");
			return NULL;
		}


		client_sd = accept(sd, (struct sockaddr*)&listen_addr, &addr_len);

		recv(client_sd, recv_buffer, 3, 0);
		if(strncmp(recv_buffer,"get", 3) == 0){

			send();
		}



	}


}