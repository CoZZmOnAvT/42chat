/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmd_helpers.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/08/03 17:34:19 by phrytsenko        #+#    #+#             */
/*   Updated: 2018/09/02 14:32:41 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"

t_dlist		*find_user_nickname(const char *nickname, t_dlist *list)
{
	t_dlist		*ret;
	t_client	*user;

	ret = list;
	while (ret && (ret = ret->next) != list)
		if ((user = ret->content) && !strcasecmp(user->nickname, nickname))
			return (ret);
	return (NULL);
}

t_dlist		*find_user_addr(void *addr, t_dlist *list)
{
	t_dlist		*ret;

	ret = list;
	while (ret && (ret = ret->next) != list)
		if (addr == ret->content)
			return (ret);
	return (NULL);
}

const char	*validate_room_data(const char **args, t_dlist **rooms,
								const t_client *client)
{
	char		*room_name;

	if (!args || !rooms || !client)
		return ("* Incorrect data passed *");
	*rooms = g_env.all_rooms;
	room_name = ft_strtrim(args[0]);
	if (!args[0] || !room_name || !*room_name)
		return ("* You sould specify room name *" - h_clean(room_name));
	while (*rooms && (*rooms = (*rooms)->next) != g_env.all_rooms)
		if (!strcasecmp(((t_chat_room *)(*rooms)->content)->name, room_name))
			break ;
	h_clean(room_name);
	if (!*rooms || *rooms == g_env.all_rooms)
		return ("* Room with this name doesn't exists *");
	else if (*rooms == client->chat_room_node)
		return ("* You are already in this room *");
	else if (!args[1] && ((t_chat_room *)(*rooms)->content)->passwd)
		return ("* Room locked by password *");
	else if (((t_chat_room *)(*rooms)->content)->passwd
				!= hash_data(args[1], ft_strlen(args[1])))
		return ("* Invalid access password to the room *");
	return (NULL);
}

const char	*new_chat_room(const char *name, const char *passwd, uint8_t unique)
{
	t_dlist		*rooms;
	t_chat_room	*new_room;

	if (!name || !*name)
		return ("* You sould specify room name *");
	else if (!nickname_is_valid(name))
		return ("* Invalid room name *");
	else if (!(new_room = ft_memalloc(sizeof(t_chat_room))))
		return ("* Server feels bad *");
	else if (ft_dlstsize(g_env.all_rooms) >= MAX_ROOMS_NUMBER)
		return ("* Maximum number of rooms created *" - h_clean(new_room));
	new_room->name = ft_strsub(name, 0, ft_cinustrcn(name, MAX_NICKNAME_LEN));
	rooms = g_env.all_rooms;
	while (rooms && (rooms = rooms->next) != g_env.all_rooms)
		if (!strcasecmp(((t_chat_room *)rooms->content)->name, new_room->name)
			&& !h_clean(new_room - h_clean(new_room->name)))
			return ("* Room with this name already exists *");
	new_room->passwd = hash_data(passwd, ft_strlen(passwd));
	sprintf(new_room->log_name, unique ? "./logs/log_chat_%s_%ld.txt"
		: "./logs/log_chat_%s.txt", new_room->name, time(NULL));
	if ((new_room->log_fd = open(new_room->log_name, O_LOG_FLAGS)) < 0)
		return ("* Server feels bad *" - h_clean(new_room));
	ft_dlstpush_back(&g_env.all_rooms, ft_dlstnew(new_room, sizeof(void *)));
	return (NULL);
}
