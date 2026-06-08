# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <strings.h>
# include <netinet/in.h>
# include <sys/select.h>
# include <sys/socket.h>

typedef struct s_client
{
	int	id;
	char	msg[110000];
}	t_client;

t_client	client[1024];
fd_set	fds, read_fds, write_fds;
int	max_fd = 0;
int	next_id = 0;
char	read_buffer[120000];
char	write_buffer[120000];

void	error()
{
	write(2, "Fatal error\n", 13);
	exit(1);
}

void	locutor(int sender)
{
	int	fd = 0;

	while (fd <= max_fd)
	{
		if (FD_ISSET(fd, &write_fds) && fd != sender)
			send(fd, write_buffer, strlen(write_buffer), 0);
		fd++;
	}
}

int	init_server(char* port)
{
	int sockfd;
	struct sockaddr_in addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error();

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(2130706433);
	addr.sin_port = htons(atoi(port));

	if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		error();
	if (listen(sockfd, 128) < 0)
		error();

	return sockfd;
}

void	handle_new_connection(int sockfd)
{
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);

	int client_fd = accept(sockfd, (struct sockaddr *)&addr, &len);
	if (client_fd < 0)
		error();
	if (client_fd > max_fd)
		max_fd = client_fd;

	client[client_fd].id = next_id++;
	FD_SET(client_fd, &fds);
	sprintf(write_buffer, "server: client %d just arrived\n", client[client_fd].id);
	locutor(client_fd);
}

void	handle_disconnect(int client_fd)
{
	sprintf(write_buffer, "server: client %d just left\n", client[client_fd].id);
	locutor(client_fd);
	FD_CLR(client_fd, &fds);
	close(client_fd);
	bzero(client[client_fd].msg, sizeof(client[client_fd].msg));
}

void	handle_message(int fd)
{
	int	i;
	int	j;
	int	bytes = recv(fd, read_buffer, sizeof(read_buffer), 0);

	if (bytes <= 0)
	{
		handle_disconnect(fd);
		return ;
	}
	i = 0;
	j = strlen(client[fd].msg);
	while (i < bytes)
	{
		client[fd].msg[j] = read_buffer[i];
		if (client[fd].msg[j] == '\n')
		{
			client[fd].msg[j] = '\0';
			sprintf(write_buffer, "client %d: %s\n", client[fd].id, client[fd].msg);
			locutor(fd);
			bzero(client[fd].msg, sizeof(client[fd].msg));
			j = -1;
		}
		i++;
		j++;
	}
}

int	main(int argc, char* argv[])
{
	if (argc != 2)
	{
		write(2, "Wrong number of arguments\n", 26);
		exit(1);
	}

	FD_ZERO(&fds);
	bzero(client, sizeof(client));
	int sockfd = init_server(argv[1]);
	max_fd = sockfd;
	FD_SET(sockfd, &fds);

	while (1)
	{
		read_fds = write_fds = fds;
		if (select(max_fd + 1, &read_fds, &write_fds, NULL, NULL) < 0)
			continue;
		int	fd = 0;
		while (fd <= max_fd)
		{
			if (!FD_ISSET(fd, &read_fds))
			{
				fd++;
				continue;
			}
			if (sockfd == fd)
				handle_new_connection(fd);
			else
				handle_message(fd);
			fd++;
		}
	}
	return 0;
}