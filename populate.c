/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   populate.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: varnaud <varnaud@student.42.us.org>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/01 21:12:15 by varnaud           #+#    #+#             */
/*   Updated: 2017/05/02 15:05:48 by varnaud          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_db.h"

void	display_entry(t_entry *entry)
{
	t_data	*c;

	printf("%s:\n", entry->id);
	c = entry->data;
	while (c)
	{
		if (!strcmp(c->key, "picture"))
			printf("%s: \033]1337;File=inline=1:%s:\a\n", c->key, c->value);
		else
			printf("%s: %s\n", c->key, c->value);
		c = c->next;
	}
	printf("\n");
}

void	display_entries(t_entry *lst)
{
	while (lst)
	{
		display_entry(lst);
		lst = lst->next;
		getchar();
	}
}

t_uid	*get_uids(void)
{
	int		fd;
	int		r;
	int		i;
	char	**av;
	char	*line;
	int		status;
	pid_t	pid;
	t_uid	*lst;
	t_uid	**cur;

	lst = NULL;
	cur = &lst;
	pid = fork();
	status = 0;
	if (pid == 0)
	{
		fd = open("tmp_uids", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
		dup2(fd, 1);
		close(fd);
		av = malloc(sizeof(char*) * 5);
		av[0] = strdup("/usr/bin/ldapsearch");
		av[1] = strdup("-x");
		av[2] = strdup("-b");
		av[3] = strdup("cn=_developer,ou=groups,dc=42,dc=us,dc=org");
		av[4] = NULL;
		execve("/usr/bin/ldapsearch", av, NULL);
		exit(1);
	}
	else if (pid > 0)
	{
		wait(&status);
		if (status)
			return (NULL);
		fd = open("tmp_uids", O_RDONLY);
		while ((r = gnl(fd, &line)) && r != -1)
		{
			av = ft_strsplit(line, ':');
			if (av)
			{
				if (av[0])
				{
					i = 0;
					if (strcmp(av[i], "memberUid") == 0 && av[i + 1] && av[i + 1][0])
					{
						*cur = malloc(sizeof(t_uid));
						(*cur)->uid = strdup(&av[i + 1][1]);
						(*cur)->next = NULL;
						cur = &(*cur)->next;
					}
					while (av[i])
						free(av[i++]);
				}
				free(av);
			}
			free(line);
		}
		close(fd);
		gnl(-42, NULL);
	}
	else
		return (NULL);
	//unlink("tmp_uids");
	return (lst);
}

int		populate(void)
{
	t_uid	*lst;
	t_uid	*cur;
	int		fd;
	pid_t	pid;
	char	**av;
	int		status;
	t_entry	*entry;
	t_entry	**curentry;

	lst = get_uids();
	if (lst == NULL)
		return (1);
	entry = NULL;
	curentry = &entry;
	while (lst)
	{
		status = 0;
		pid = fork();
		if (pid == 0)
		{
			fd = open("tmp_uidinfo", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
			dup2(fd, 1);
			close(fd);
			av = malloc(sizeof(char*) * 4);
			av[0] = strdup("/usr/bin/ldapsearch");
			av[1] = strdup("-x");
			av[2] = ft_strjoin("uid=", lst->uid);
			av[3] = NULL;
			execve("/usr/bin/ldapsearch", av, NULL);
			exit(1);
		}
		else if (pid > 0)
		{
			wait(&status);
			if (status)
				return (-1);
			printf("Getting entry of: %s\n", lst->uid);
			*curentry = get_entry();
			display_entry(*curentry);
			getchar();
			curentry = &(*curentry)->next;
		}
		else
			return (-1);
		cur = lst;
		lst = lst->next;
		free(cur->uid);
		free(cur);
	}
	//display_entries(entry);
	return (0);
}

int		main(void)
{
	if (populate())
	{
		printf("RIP\n");
		exit(1);
	}
}
