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

#define MAX_CONNECTIONS 10
#define PORT 31337

void setnonblocking(int sock);
void build_select_list();
void handle_new_connection();
void handle_data(int i);
void masterfl_insert();
void masterfl_remove(int i);





