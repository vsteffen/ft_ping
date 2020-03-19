#include "ft_ping.h"

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
	uint16_t	*buff;
	uint32_t	sum;

	buff = (uint16_t *)addr;
	for (sum = 0; size > 1; size -= 2)
		sum += *buff++;
	if (size == 1)
		sum += *(uint8_t*)buff;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	return (~sum);
}

void	fill_ping_pkt4(struct s_ping_pkt4 *ping_pkt, time_t timestamp) {
	ft_bzero(ping_pkt, sizeof(struct s_ping_pkt4));

	ping_pkt->icmp.icmp_type = ICMP_ECHO;
	ping_pkt->icmp.icmp_code = PING_ICMP_ECHO_CODE;
	ping_pkt->icmp.icmp_id = BSWAP16((uint16_t)getpid());
	g_ping->stat.icmp_seq_send++;
	ping_pkt->icmp.icmp_seq = BSWAP16(g_ping->stat.icmp_seq_send);
	ping_pkt->timestamp = BSWAP64(timestamp);
	ft_memcpy(&ping_pkt->icmp.icmp_dun, &timestamp, sizeof(timestamp));
	ping_pkt->icmp.icmp_cksum = checksum(ping_pkt, sizeof(*ping_pkt));
}

void	fill_reply(struct s_reply *reply) {
	reply->msg.msg_name = NULL;
	reply->msg.msg_iov = &reply->iov;
	reply->iov.iov_base = reply->recv_buff;
	reply->iov.iov_len = sizeof(reply->recv_buff);
	reply->msg.msg_iovlen = 1;
	reply->msg.msg_control = NULL;
	reply->msg.msg_controllen = 0;
}

void	read_ping_4(struct s_reply *reply) {
	struct ip	*ip_hdr;
	ssize_t		ip_icmp_offset;

	ip_hdr = (struct ip *)reply->recv_buff;
	ip_icmp_offset = ip_hdr->ip_hl << 2;
	if (ip_hdr->ip_p != IPPROTO_ICMP || (size_t)reply->read_bytes < ip_icmp_offset + sizeof(struct icmp)) {
		dprintf(STDERR_FILENO, "%s: ICMP header from echo reply truncated\n", PROG_NAME);
		exit_clean(EXIT_FAILURE);
	}
	reply->icmp_hdr.icmp4 = (struct icmp *)(reply->recv_buff + ip_icmp_offset);
}

void	ping_loop(int family)
{
	union u_ping_pkt	ping_pkt;
	struct timeval		tv_seq_start;
	struct s_reply 		reply;

	if (family == AF_INET6) {
		printf("TODO IPv6\n");
		exit_clean(EXIT_FAILURE);
	}

	printf("PING %s (%s) %zu(%d) bytes of data.\n", g_ping->dest.hostname, g_ping->dest.ip, PING_PKT_DATA_SIZE + sizeof(struct ip), PING_PKT_SIZE);

	g_ping->sock_fd = init_socket();
	if (gettimeofday((struct timeval *)&g_ping->stat.tv_ping_start, NULL) == -1)
		PERROR("gettimeofday");

	while (true) {
		if (gettimeofday(&tv_seq_start, NULL) == -1)
			PERROR("gettimeofday");
		if (family == AF_INET)
			fill_ping_pkt4(&ping_pkt.pkt4, tv_seq_start.tv_sec);
		else
			exit_clean(EXIT_FAILURE); // TODO IPv6
		g_ping->wait_alarm = true;
		alarm(g_ping->interval);
		if (family == AF_INET) {
			if (sendto(g_ping->sock_fd, &ping_pkt.pkt4, sizeof(ping_pkt.pkt4), 0, (struct sockaddr*)&g_ping->dest.sa_in.ip4, sizeof(g_ping->dest.sa_in.ip4)) == -1)
				PERROR("sendto");
		}
		else {
			exit_clean(EXIT_FAILURE); // TODO IPv6
		}

		fill_reply(&reply);

		if ((reply.read_bytes = recvmsg(g_ping->sock_fd, &reply.msg, 0)) == -1) {
			if (errno == EINTR)
				dprintf(STDERR_FILENO, "%s: Request timed out\n", PROG_NAME);
			// TODO If timed out
			else
				PERROR("recvmsg");
		}
		if (reply.read_bytes == 0) {
			dprintf(STDERR_FILENO, "%s: socket closed\n", PROG_NAME);
			exit_clean(EXIT_FAILURE);
		}
		// printf("Receive data bytes = %zd\n", reply.read_bytes);
		if ((size_t)reply.read_bytes < sizeof(struct ip)/* || TODO IPv6 check*/) {
			dprintf(STDERR_FILENO, "%s: IP header from echo reply truncated\n", PROG_NAME);
			exit_clean(EXIT_FAILURE);
		}

		if (family == AF_INET) {
			read_ping_4(&reply);
			print_ping_4(&reply, &tv_seq_start);
		}
		else {
			exit_clean(EXIT_FAILURE); // TODO IPv6
		}

		if (gettimeofday((struct timeval *)&g_ping->stat.tv_ping_end, NULL) == -1)
			PERROR("gettimeofday");
		while (g_ping->wait_alarm) ;
	}
}
