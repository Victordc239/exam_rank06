#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>

typedef struct s_client
{
	int id;
	char msg[110000];
}	t_client;

t_client	client[1024];
fd_set	fds, write_fds, read_fds;

int next_id = 0;
int max_fd = 0;
char write_buffer[120000];
char read_buffer[120000];

void	error()
{
	write(2, "Fatal error\n", 13);
	exit(1);
}

