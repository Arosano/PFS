#include "client_PFS.h"

char file_list[1024];

int main(int argc, char *argv[]){

	pthread_t get_thread;
	pthread_create(&get_thread, &attr, handle_get, (void *) NULL);

	


}