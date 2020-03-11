#include "ft_ping.h"

volatile struct s_ping	*g_ping;

void	exit_clean(int exit_status) {
	if (g_ping->dest.reverse_dns)
		free(g_ping->dest.reverse_dns);
	if (g_ping->sock_fd != -1)
		close(g_ping->sock_fd);
	exit(exit_status);
}

int	init_socket(void)
{
	int	sock_fd;

	if ((sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
		PERROR("socket");
	if (setsockopt(sock_fd, IPPROTO_IP, IP_TTL, (const void *)&g_ping->ttl, sizeof(g_ping->ttl)) == -1)
		PERROR("setsockopt");
	setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const void *)&g_ping->tv_timeout, sizeof(g_ping->tv_timeout));
	return (sock_fd);
}

uint16_t	checksum(void *addr, int size) {
	uint16_t	*buff = (uint16_t *)addr;
	uint32_t	sum;

	for (sum = 0; size > 1; size -= 2)
		sum += *buff++;
	if (size == 1)
		sum += *(uint8_t*)buff;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	return (~sum);
}

void	fill_ping_packet4(struct s_ping_pkt4 *ping_packet) {
	ft_bzero(ping_packet, sizeof(struct s_ping_pkt4));

	ping_packet->icmp.type = ICMP_ECHO;
	ping_packet->icmp.code = ICMP_ECHO_CODE;
	ping_packet->icmp.un.echo.id = ICMP_ECHO_ID;
	ping_packet->icmp.un.echo.sequence = g_ping->stat.icmp_seq_send++;
	ping_packet->icmp.checksum = checksum(ping_packet, sizeof(struct s_ping_pkt4));
}

void	print_statistics() {
	float	rate;
	double	total_time_elapsed;
	double	rtt_mean;
	double	rtt_mdev;

	printf("\n--- %s ping statistics ---\n", g_ping->dest.hostname);
	rate = ABS((float)(g_ping->stat.icmp_seq_recv / (float)g_ping->stat.icmp_seq_send) * 100.0 - 100.0);
	if (g_ping->stat.icmp_seq_send == 1)
		total_time_elapsed = 0.0;
	else
		total_time_elapsed = (double)(g_ping->stat.tv_ping_end.tv_sec - g_ping->stat.tv_ping_start.tv_sec) * 1000.0 + (double)(g_ping->stat.tv_ping_end.tv_usec - g_ping->stat.tv_ping_start.tv_usec) / 1000.0;
	printf("%zu packets transmitted, %zu received, %.0lf%% packet loss, time %.0lfms\n", g_ping->stat.icmp_seq_send, g_ping->stat.icmp_seq_recv, rate, total_time_elapsed);
	if (g_ping->stat.icmp_seq_recv == g_ping->stat.icmp_seq_send) {
		rtt_mean = g_ping->stat.rtt_sum / g_ping->stat.icmp_seq_recv;
		rtt_mdev = ft_sqrt_newton(g_ping->stat.rtt_square_sum / g_ping->stat.icmp_seq_recv - ft_pow(rtt_mean, 2));
		printf("rtt min/avg/max/mdev = %.3lf/%.3lf/%.3lf/%.3lf ms\n", g_ping->stat.rtt_min, rtt_mean, g_ping->stat.rtt_max, rtt_mdev);
	}
}

void	update_stats(double tv_seq_diff) {
	if (g_ping->stat.icmp_seq_recv == 1)
		g_ping->stat.rtt_min = tv_seq_diff;
	else {
		if (g_ping->stat.rtt_min > tv_seq_diff)
			g_ping->stat.rtt_min = tv_seq_diff;
	}
	if (g_ping->stat.rtt_max < tv_seq_diff)
		g_ping->stat.rtt_max = tv_seq_diff;
	g_ping->stat.rtt_sum += tv_seq_diff;
	g_ping->stat.rtt_square_sum += tv_seq_diff * tv_seq_diff;
}

void	send_ping4(struct sockaddr_in *ping_addr, char *ping_dom, char *ping_ip, char *rev_host)
{
	struct s_ping_pkt4	ping_packet;
	struct sockaddr_in	r_addr;
	struct timeval		tv_seq_start;
	struct timeval		tv_seq_end;

	printf("PING %s (%s) ?(?) bytes of data.\n", ping_dom, ping_ip);

	g_ping->sock_fd = init_socket();
	if (gettimeofday((struct timeval *)&g_ping->stat.tv_ping_start, NULL) == -1)
		PERROR("gettimeofday");
	while(true) {
		fill_ping_packet4(&ping_packet);
		if (gettimeofday(&tv_seq_start, NULL) == -1)
			PERROR("gettimeofday");
		alarm(g_ping->interval);
		g_ping->wait_alarm = true;
		if (sendto(g_ping->sock_fd, &ping_packet, sizeof(ping_packet), 0, (struct sockaddr*)ping_addr, sizeof(*ping_addr)) == -1)
			PERROR("sendto");
		socklen_t addr_len = sizeof(r_addr);
		if (recvfrom(g_ping->sock_fd, &ping_packet, sizeof(ping_packet), 0, (struct sockaddr*)&r_addr, &addr_len) == -1)
			PERROR("recvfrom");
		// if (ping_packet.icmp.type != ICMP_ECHOREPLY) {
		// 	printf("ECHO REPLY error: ICMP type %hhu and code %hhu\n", ping_packet.icmp.type, ping_packet.icmp.code);
		// }
		// else {
			g_ping->stat.icmp_seq_recv++;
			if (gettimeofday(&tv_seq_end, NULL) == -1)
				PERROR("gettimeofday");
			double tv_seq_diff = (double)(tv_seq_end.tv_sec - tv_seq_start.tv_sec) * 1000.0 + (double)(tv_seq_end.tv_usec - tv_seq_start.tv_usec) / 1000.0;
			printf("%d bytes from %s (%s): icmp_seq=%zu ttl=%d time=%.2lf ms\n", 64, rev_host, ping_ip, g_ping->stat.icmp_seq_send, g_ping->ttl, tv_seq_diff);
			update_stats(tv_seq_diff);
		// }
		if (gettimeofday((struct timeval *)&g_ping->stat.tv_ping_end, NULL) == -1)
			PERROR("gettimeofday");
		while (g_ping->wait_alarm) ;
	}
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
			break ; // remove to handle IPv6
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
	ping->ttl = TTL;
	ping->dest.reverse_dns = NULL;
	ping->sock_fd = -1;
	ping->tv_timeout.tv_sec = 0;
	ping->tv_timeout.tv_usec = DEFAULT_TIMEOUT;
	ping->interval = DEFAULT_INTERVAL;
	ft_bzero(&ping->stat, sizeof(ping->stat));

	get_dest_ip(av[1], ping);
	ping->dest.reverse_dns = reverse_dns_lookup(ping->dest.ip, ping->dest.family);
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
	if (ping.dest.family == AF_INET)
		send_ping4(&ping.dest.sa_in.ip4, ping.dest.hostname, ping.dest.ip, (ping.dest.reverse_dns ? ping.dest.reverse_dns : ping.dest.ip));
	else
		printf("Ping IPv6 function\n");
	exit_clean(EXIT_FAILURE);
}
