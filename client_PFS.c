#include "client_PFS.h"

char file_list[1024];

int main(int argc, char *argv[]){

	char client_id = *argv[1];


	build_file_list();

	pthread_t get_thread;
	pthread_create(&get_thread, &attr, handle_get, (void *) NULL);
	

	register_with_server();


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
	    sprintf (&file_list[bytes_stored],"%s, ", ent->d_name);
	    bytes_stored += strlen(ent->d_name);
	    /*open file, get size, store size in file list*/
	    fp = fopen (ent->d_name, r+);
	    fseek(fp, 0L, SEEK_END);
		sz = ftell(fp);
		sprintf(&file_list[bytes_stored],"%d, ", sz);

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

			send(client_sd, file_list, strlen(file_list), 0);
		}

		close(client_sd);



	}


}