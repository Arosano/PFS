#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <dirent.h>

pthread_attr_t attr;

void* handle_inc_get();
void handle_get_command(char* data);
void build_file_list();
void* update_recv(void* sock);

void setnonblocking(int sock);

