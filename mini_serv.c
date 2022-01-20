#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>

typedef struct s_list {
	int id;
	int fd;
	char *read_buf;
	char *write_buf;
	struct s_list *next;
} t_list;

t_list *g_begin = NULL;
int		g_sockfd = -1;

t_list *lst_create(int fd)
{
	static int id = 0;
	t_list *client = (t_list *)malloc(sizeof(t_list));
	if (client)
	{
		client->id = id;
		id++;
		client->fd = fd;
		client->read_buf = NULL;
		client->write_buf = NULL;
		client->next = NULL;
	}
	return client;
}

void lst_push(t_list *client)
{
	client->next = g_begin;
	g_begin = client;
}

void lst_del_one(t_list *client)
{
	if (client->fd >= 0)
		close(client->fd);
	if (client->read_buf)
		free(client->read_buf);
	if (client->write_buf)
		free(client->write_buf);
	free(client);
}

void lst_clear()
{
	t_list *itr = g_begin;
	t_list *tmp;
	while (itr)
	{
		tmp = itr;
		itr = itr->next;
		lst_del_one(tmp);
	}
	g_begin = NULL;
}

void lst_remove_one(t_list *client)
{
	t_list *itr = g_begin;

	if (!itr)
		return;
	if (itr == client)
	{
		g_begin = itr->next;
		lst_del_one(client);
		return;
	}
	while (itr->next)
	{
		if (itr->next == client)
		{
			itr->next = itr->next->next;
			lst_del_one(client);
			return;
		}
		itr = itr->next;
	}
}

int fatal_error()
{
	if (g_sockfd >= 0)
		close(g_sockfd);
	lst_clear();
	char *msg = "Fatal error\n";
	write(2, msg, strlen(msg));
	return 1;
}



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

int add_msg_to_all(int skip_id, char *msg)
{
	t_list *itr = g_begin;
	while (itr)
	{
		if (itr->id != skip_id)
		{
			char *tmp = str_join(itr->write_buf, msg);
			if (!tmp)
				return 0;
			itr->write_buf = tmp;
		}
		itr = itr->next;
	}
	return 1;
}

int broadcast_msg(int id, char *msg)
{
	char *buf = (char *)malloc(sizeof(char) * (strlen(msg) + 50));
	if (!buf)
		return 0;
	int len = sprintf(buf, "client %d: %s", id, msg);
	buf[len] = '\0';
	if (!add_msg_to_all(id, buf))
	{
		free(buf);
		return 0;
	}
	free(buf);
	return 1;
}

int broadcast_join(int id)
{
	char buf[50];
	int len = sprintf(buf, "server: client %d just arrived\n", id);
	buf[len] = '\0';
	if (!add_msg_to_all(id, buf))
		return 0;
	return 1;
}

int broadcast_leave(int id)
{
	char buf[50];
	int len = sprintf(buf, "server: client %d just left\n", id);
	buf[len] = '\0';
	if (!add_msg_to_all(id, buf))
		return 0;
	return 1;
}

int main(int ac, char **av) {
	socklen_t len;
	struct sockaddr_in servaddr, cli;

	if (ac < 2)
	{
		char *msg = "Wrong number of arguments\n";
		write(2, msg, strlen(msg));
		return 1;
	}

	g_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (g_sockfd == -1) {
		return fatal_error();
	}
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(av[1]));

	// Binding newly created socket to given IP and verification
	if ((bind(g_sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) {
		return fatal_error();
	}
	if (listen(g_sockfd, 10) != 0) {
		return fatal_error();
	}

	fd_set fdset;
	fd_set read_fds;
	fd_set write_fds;

	FD_ZERO(&fdset);
	FD_SET(g_sockfd, &fdset);

	char recv_buf[1024];

	while (1)
	{
		read_fds = fdset;
		write_fds = fdset;

		if (select(FD_SETSIZE, &read_fds, &write_fds, NULL, NULL) < 0)
			return fatal_error();
		if (FD_ISSET(g_sockfd, &read_fds))
		{
			len = sizeof(cli);
			int connfd = accept(g_sockfd, (struct sockaddr *)&cli, &len);
			if (connfd < 0) {
    			return fatal_error();
			}
			t_list *new_client = lst_create(connfd);
			if (!new_client)
			{
				close(connfd);
				return fatal_error();
			}
			lst_push(new_client);
			if (!broadcast_join(new_client->id))
				return fatal_error();
			FD_SET(connfd, &fdset);
		}
		t_list *itr = g_begin;
		while (itr)
		{
			if (FD_ISSET(itr->fd, &read_fds))
			{
				int recv_ret = recv(itr->fd, recv_buf, 1023, 0);
				if (recv_ret <= 0)
				{
					FD_CLR(itr->fd, &fdset);
					if (!broadcast_leave(itr->id))
						return fatal_error();
					t_list *tmp = itr;
					itr = itr->next;
					lst_remove_one(tmp);
					continue;
				}
				recv_buf[recv_ret] = '\0';
				char *tmp = str_join(itr->read_buf, recv_buf);
				if (!tmp)
					return fatal_error();
				itr->read_buf = tmp;
				int ext_ret;
				char *ext_msg = NULL;
				while ((ext_ret = extract_message(&(itr->read_buf), &ext_msg)))
				{
					if (ext_ret == -1)
						return fatal_error();
					if (!broadcast_msg(itr->id, ext_msg))
					{
						free(ext_msg);
						return fatal_error();
					}
					free(ext_msg);
					ext_msg = NULL;
				}
			}
			itr = itr->next;
		}
		itr = g_begin;
		while (itr)
		{
			if (FD_ISSET(itr->fd, &write_fds) && itr->write_buf)
			{
				int size = strlen(itr->write_buf);
				int send_ret = send(itr->fd, itr->write_buf, size, 0);
				if (send_ret == size)
				{
					free(itr->write_buf);
					itr->write_buf = NULL;
				}
				else if (send_ret > 0)
					strcpy(itr->write_buf, itr->write_buf + send_ret);
			}
			itr = itr->next;
		}
	}
	return 0;
}
