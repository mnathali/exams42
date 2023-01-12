#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}

int	init_server(int port, struct sockaddr_in *servaddr)
{
	int sockfd = -1;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd == -1)
		return -1; 
	
	bzero(servaddr, sizeof(*servaddr)); 

	// assign IP, PORT 
	servaddr->sin_family = AF_INET; 
	servaddr->sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr->sin_port = htons(port); 
  
	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (const struct sockaddr *)servaddr, sizeof(*servaddr))) != 0)
		return -1; 

	if (listen(sockfd, 20) != 0)
		return -1; 
		
	return sockfd;
}


int main(int argc, char **argv)
{
	int sockfd, connfd, id, n, max;
	int ids[21];//check FOPEN_MAX and try to increase array size if fails
	struct sockaddr_in servaddr; 
	fd_set	rd, wr, cp;
	char *message = 0;
	char buf[1024];

	if (argc != 2)
	{
		write(2, "Wrong number of arguments\n", 26);
		return 1;
	}
	// socket create and verification 
	sockfd = init_server(atoi(argv[1]), &servaddr);
	if (sockfd == -1)
	{
		write(2, "Fatal error\n", 12); 
		return 1; 
	}
	FD_ZERO(&cp);
	FD_SET(sockfd, &cp);
	max = sockfd;
	id = 0;
	while (1)
	{
		rd = wr = cp;
		select(max + 1, &rd, &wr, NULL, NULL);

		if (FD_ISSET(sockfd, &rd))
		{
			connfd = accept(sockfd, NULL, NULL);
			FD_SET(connfd, &cp);
			ids[connfd] = id++;
			sprintf(buf, "server: client %d just arrived\n", ids[connfd]);
			for (int i = 3; i <= max; i++)
				if (FD_ISSET(i, &wr) && i != connfd)
					send(i, buf, strlen(buf), 0);
			if (connfd > max)
				max = connfd;
		}
		for (int i = 0; i <= max; ++i)
		{
			if (!FD_ISSET(i, &rd) || i == sockfd)
				continue;
			while ((n = recv(i, buf, 1024 - 1, 0)) > 0)//it is possible to read in one system call on exam with buffer size 1000 to make program non-blocking
			{
				buf[n] = 0;
				message = str_join(message, buf);
				if (n < 1024 - 1)
					break ;
			}
			if (n <= 0)
			{
				sprintf(buf, "server: client %d just left\n", ids[i]);
				FD_CLR(i, &cp);
				close(i);
				for (int j = 0; j <= max; j++)
					if (FD_ISSET(j, &wr) && j != i)
						send(j, buf, strlen(buf), 0);
			}
			else
			{
				char *tmp;
				sprintf(buf, "client %d: ", ids[i]);
				while (extract_message(&message, &tmp))
				{
					for (int j = 0; j <= max; j++)
					{
						if (FD_ISSET(j, &wr) && j != i)
						{
							send(j, buf, strlen(buf), 0);
							send(j, tmp, strlen(tmp), 0);
						}
					}
					free(tmp);
				}
			}
			if (message)
			{
				free(message);
				message = 0;
			}
		}
	}
	return 0;
}