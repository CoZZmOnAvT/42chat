/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/07/30 13:18:50 by pgritsen          #+#    #+#             */
/*   Updated: 2018/08/26 21:40:30 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.h"

t_env			g_env;

void			proceed_cmds(t_command cmd, char * data)
{
	if (cmd == UPDATE_USERS)
		update_users_online(data);
	else if (cmd == UPDATE_ROOMS)
		update_rooms(data);
	else if (cmd == UPDATE_HISTORY)
		update_chat_history(data);
}

void			get_messages(int fd, short ev, void *data)
{
	char			* buffer;
	char			* point;
	t_dlist			* lines;
	t_dlist			* tmp;
	t_command		cmd;

	(void)ev;
	(void)data;
	if (recieve_data(fd, (void **)&buffer, &cmd, MSG_WAITALL) <= 0)
		return ;
	else if (cmd != NO_CMD)
		return (proceed_cmds(cmd, buffer));
	lines = ft_strsplit_dlst(buffer, '\n');
	free(buffer);
	tmp = lines;
	while (tmp && (tmp = tmp->next) != lines)
		if ((point = ft_strchr(tmp->content, '\a')))
		{
			g_env.layot.chat_offset = 0;
			beep();
			ft_strclr(point);
		}
	g_env.chat_history.size += ft_dlstsize(lines);
	ft_dlstmerge(&g_env.chat_history.lines, &lines);
	display_chat();
}

char			*init_socket(struct sockaddr_in * c_data, const char * server_ip)
{
	if ((g_env.sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return ("Unable to create socket");
	memset(c_data, '0', sizeof(*c_data));
	c_data->sin_family = AF_INET;
	c_data->sin_port = htons(PORT);
	if(inet_pton(AF_INET, server_ip, &c_data->sin_addr) <= 0)
		return ("Invalid address");
	else if (connect(g_env.sockfd, (struct sockaddr *)c_data, sizeof(*c_data)) < 0)
		return ("Couldn't connect to server, try again later.\n");
	return (0);
}

int				try_reconnect(void)
{
	close(g_env.sockfd);
	if ((g_env.sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return (-1);
	else if (connect(g_env.sockfd, (struct sockaddr *)&g_env.conn_data,
						sizeof(g_env.conn_data)) < 0)
		return (-1);
	else if (send_data(g_env.sockfd, g_env.nickname,
						ft_strlen(g_env.nickname) + 1, 0) < 0)
		return (-1);
	event_del(&g_env.ev_getmsg);
	event_set(&g_env.ev_getmsg, g_env.sockfd, EV_READ | EV_PERSIST,
				get_messages, NULL);
	event_add(&g_env.ev_getmsg, NULL);
	ft_memdel((void**)&g_env.nickname);
	recieve_data(g_env.sockfd, (void **)&g_env.nickname, 0, MSG_WAITALL);
	return (1);
}

int				main(int ac, char **av)
{
	char		*err;

	setlocale(LC_ALL, "");
	bzero(&g_env, sizeof(t_env));
	if (ac < 2)
		return (ft_printf("Usage: ./42chat [server_ip_address]\n") * 0);
	else if ((err = init_socket(&g_env.conn_data, av[1])))
		return (ft_printf("%s\n", err) * 0 + EXIT_FAILURE);
	signal(SIGPIPE, SIG_IGN);
	event_init();
	init_design();
	handle_input(0, 0, true);
	nodelay(g_env.ws.input, true);
	event_set(&g_env.ev_getmsg, g_env.sockfd,
				EV_READ | EV_PERSIST, get_messages, NULL);
	event_add(&g_env.ev_getmsg, NULL);
	event_set(&g_env.ev_input, 0, EV_WRITE | EV_PERSIST,
				(void (*)(int, short, void *))handle_input, NULL);
	event_add(&g_env.ev_input, NULL);
	event_loop(0);
	curses_exit(NULL, NULL);
	return (0);
}
