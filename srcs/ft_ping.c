#include "ft_ping.h"

volatile struct s_ping	*g_ping;

void	exit_clean(int exit_status) {
	if (g_ping->dest.reverse_dns)
		free(g_ping->dest.reverse_dns);
	if (g_ping->sock_fd != -1)
		close(g_ping->sock_fd);
	exit(exit_status);
}

char	*reverse_dns_lookup(char *ip_str, int family) {
	t_sa_in	sa_in;
	char	buff_hostname[NI_MAXHOST];
	int	status;

	if ((status = inet_pton(family, ip_str, (family == AF_INET ? (void *)&sa_in.ip4.sin_addr : (void *)&sa_in.ip6.sin6_addr))) == -1)
		PERROR("inet_pton");
	if (status == 0) {
		dprintf(STDERR_FILENO, "%s: inet_pton: Invalid string\n", PROG_NAME);
		exit_clean(EXIT_FAILURE);
	}
	if (family == AF_INET) {
		sa_in.ip4.sin_family = family;
		sa_in.ip4.sin_port = 0;
	}
	else {
		sa_in.ip6.sin6_family = family;
		sa_in.ip6.sin6_port = 0;
	}

	if ((status = getnameinfo((family == AF_INET ? (struct sockaddr *)&sa_in.ip4 : (struct sockaddr *)&sa_in.ip6), (family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6)), buff_hostname, sizeof(buff_hostname), NULL, 0, NI_NAMEREQD)) != 0) {
		dprintf(STDERR_FILENO, "%s: getnameinfo: %s\n", PROG_NAME, gai_strerror(status)); // not authorized by the subject but it would be stupid to not do it
		return (NULL);
	}
	return (ft_strdup(buff_hostname));
}

void	get_dest_ip(char *dest, struct s_ping *ping) {
	struct addrinfo	hints;
	struct addrinfo	*res;
	struct addrinfo	*tmp_ptr;
	int		status;
	const char	*tmp_str = NULL;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((status = getaddrinfo(dest, NULL, &hints, &res)) != 0) {
		dprintf(STDERR_FILENO, "%s: getaddrinfo: %s\n", PROG_NAME, gai_strerror(status)); // not authorized by the subject but it would be stupid to not do it
		exit_clean(EXIT_FAILURE);
	}

	tmp_ptr = res;
	while (tmp_ptr) {
		if (tmp_ptr->ai_family == AF_INET) {
			memcpy(&(ping->dest.sa_in.ip4), (void *)tmp_ptr->ai_addr, sizeof(struct sockaddr_in));
			tmp_str = inet_ntop(AF_INET, &(ping->dest.sa_in.ip4.sin_addr), ping->dest.ip, INET_ADDRSTRLEN);
			ping->dest.family = AF_INET;
			break ;
		}
		else if (tmp_ptr->ai_family == AF_INET6) {
			memcpy(&(ping->dest.sa_in.ip6), (void *)tmp_ptr->ai_addr, sizeof(struct sockaddr_in6));
			tmp_str = inet_ntop(AF_INET6, &(ping->dest.sa_in.ip6.sin6_addr), ping->dest.ip, INET6_ADDRSTRLEN);
			ping->dest.family = AF_INET6;
			// break ; // remove to handle IPv6
		}
		tmp_ptr = tmp_ptr->ai_next;
	}
	freeaddrinfo(res); // not authorized by the subject but it would be stupid to not do it
	if (!tmp_str)
		PERROR("inet_ntop");
	if (!tmp_ptr) {
		dprintf(STDERR_FILENO, "%s: %s: No address associated with hostname\n", PROG_NAME, dest);
		exit_clean(EXIT_FAILURE);
	}
}

void	init_struct_ping(struct s_ping *ping, char **av) {
	g_ping = ping;
	ping->dest.hostname = av[1];
	ping->ttl = DEFAULT_TTL;
	ping->dest.reverse_dns = NULL;
	ping->sock_fd = -1;
	ping->tv_timeout.tv_sec = DEFAULT_TIMEOUT / 1000;
	ping->tv_timeout.tv_usec = DEFAULT_TIMEOUT * 1000 % 1000000;
	ping->interval = DEFAULT_INTERVAL;
	ft_bzero(&ping->stat, sizeof(ping->stat));
	get_dest_ip(av[1], ping);
	ping->dest.reverse_dns = reverse_dns_lookup(ping->dest.ip, ping->dest.family);
	if (!ping->dest.reverse_dns)
		ping->dest.reverse_dns = ft_strdup(ping->dest.ip);
}

int	main(int ac, char **av)
{
	struct s_ping	ping;

	if (ac < 2) {
		dprintf(STDERR_FILENO, USAGE, PROG_NAME);
		return 1;
	}
	init_struct_ping(&ping, av);
	signal(SIGINT, &signal_handler_int);
	signal(SIGALRM, &signal_handler_alrm);
	ping_loop(ping.dest.family);
}
