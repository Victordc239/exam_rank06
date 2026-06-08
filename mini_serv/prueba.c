#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <string.h>
#include <strings.h>
#include <netinet/in.h>

typedef struct s_client
{
	int id;
	char msg[110000];
}	t_client;

t_client	client[1024];
fd_set	fds, write_fds, read_fds;

int next_id = 0;
int max_fd = 0;
char read_buffer[120000];
char write_buffer[120000];

void error()
{
	write(2, "Fatal error\n", 13);
	exit(1);
}

void locutor(int sender)
{
	int fd = 0;

	while (fd <= max_fd)
	{
		if (FD_ISSET(fd, &write_fds) && sender != fd)
			send(fd, write_buffer, sizeof(write_buffer), 0);
		fd++;
	}
}

int initserver(char *port)
{
	int sockfd;
	struct sockaddr_in addr;

	sockfd = socket(2, SOCK_STREAM, 0);
	if(sockfd < 0)
		error();
	
	addr.sin_family = 2;
	addr.sin_addr.s_addr = htons(2130706433);
	addr.sin_port = htons(atoi(port));

	if(bind(sockfd, (struct sockaddr_in *)&addr, sizeof(addr)) < 0)
		error();
	if(listen(sockfd, 128) < 0)
		error();
	return sockfd;
}

void new_connection(int sockfd)
{
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);

	int client_fd = accept(sockfd, (struct sockaddr_in *) &addr, &len);
	if (client_fd < 0)
		error;
	if(client_fd > max_fd)
		client_fd = max_fd;
	
	client[client_fd].id = next_id++;
	FD_SET(client_fd, &fds);
	sprintf(write_buffer, "server: client %d just arrived\n", client[client_fd].id);
	locutor(client_fd);
}

void disconnect(int client_fd)
{
	sprintf(write_buffer, "server: client %d just left\n", client[client_fd].id);
	locutor(client_fd);
	FD_CLR(client_fd, &fds);
	close(client_fd);
	bzero(client[client_fd].msg, sizeof(client[client_fd].msg));
}