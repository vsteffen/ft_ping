#include "ft_ping.h"

int	init_socket(int family)
{
	int	sock_fd;

	if (family == AF_INET) {
		if ((sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
			PERROR("socket");
		if (setsockopt(sock_fd, IPPROTO_IP, IP_TTL, (const void *)&g_ping->options.ttl, sizeof(g_ping->options.ttl)) == -1)
			PERROR("setsockopt");
	}
	else {
		if ((sock_fd = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6)) == -1)
			PERROR("socket");
		int flag_on = 1;
		if (setsockopt(sock_fd, IPPROTO_IPV6, IPV6_RECVHOPLIMIT, &flag_on, sizeof(flag_on)) == -1)
			PERROR("setsockopt");
	}
	if (g_ping->options.set[e_option_timeout]) {
		if (setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const void *)&g_ping->options.timeout, sizeof(g_ping->options.timeout)) == -1)
			PERROR("setsockopt");
	}
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

void	fill_ping_pkt_4(struct s_ping_pkt4 *ping_pkt, time_t timestamp) {
	ft_bzero(ping_pkt, sizeof(struct s_ping_pkt4));

	ping_pkt->icmp.icmp_type = ICMP_ECHO;
	ping_pkt->icmp.icmp_code = PING_ICMP_ECHO_CODE;
	ping_pkt->icmp.icmp_id = BSWAP16((uint16_t)getpid());
	g_ping->stat.icmp_send++;
	ping_pkt->icmp.icmp_seq = BSWAP16(g_ping->stat.icmp_send);
	ping_pkt->timestamp = BSWAP64(timestamp);
	ft_memcpy(&ping_pkt->icmp.icmp_dun, &timestamp, sizeof(timestamp));
	ping_pkt->icmp.icmp_cksum = checksum(ping_pkt, sizeof(*ping_pkt));
}

void	fill_ping_pkt_6(struct s_ping_pkt6 *ping_pkt, time_t timestamp) {
	ft_bzero(ping_pkt, sizeof(struct s_ping_pkt6));

	ping_pkt->icmp.icmp6_type = ICMP6_ECHO_REQUEST;
	ping_pkt->icmp.icmp6_code = PING_ICMP_ECHO_CODE;
	ping_pkt->icmp.icmp6_id = BSWAP16((uint16_t)getpid());
	g_ping->stat.icmp_send++;
	ping_pkt->icmp.icmp6_seq = BSWAP16(g_ping->stat.icmp_send);
	ping_pkt->timestamp = BSWAP64(timestamp);
	ping_pkt->icmp.icmp6_cksum = checksum(ping_pkt, sizeof(*ping_pkt));
}

void	fill_send_msg(struct s_reply *reply, struct s_ping_pkt6 *pkt6) {
	reply->msg.msg_name = (void *)&g_ping->dest.sa_in.ip6;
	reply->msg.msg_namelen = sizeof(g_ping->dest.sa_in.ip6);
	reply->msg.msg_iov = &reply->iov;
	reply->iov.iov_base = pkt6;
	reply->iov.iov_len = sizeof(pkt6);
	reply->msg.msg_iovlen = 1;

	reply->msg.msg_control = &reply->ctrl.buff;
	reply->msg.msg_controllen = sizeof(reply->ctrl.buff);

	struct cmsghdr *cmsg;
	cmsg = CMSG_FIRSTHDR(&reply->msg);
	cmsg->cmsg_level = IPPROTO_IPV6;
	cmsg->cmsg_type = IPV6_HOPLIMIT;
	cmsg->cmsg_len = CMSG_LEN(sizeof(int));
	memcpy(CMSG_DATA(cmsg), (void *)&g_ping->options.ttl, sizeof(int));
}

void	fill_recv_msg(struct s_reply *reply) {
	reply->msg.msg_name = NULL;
	reply->msg.msg_iov = &reply->iov;
	reply->iov.iov_base = reply->recv_buff;
	reply->iov.iov_len = sizeof(reply->recv_buff);
	reply->msg.msg_iovlen = 1;
	reply->msg.msg_control = &reply->ctrl;
	reply->msg.msg_controllen = sizeof(reply->ctrl);
}

void	read_ping_4(struct s_reply *reply) {
	struct ip	*ip_hdr;
	ssize_t		ip_icmp_offset;

	if ((size_t)reply->read_bytes < sizeof(struct ip)) {
		dprintf(STDERR_FILENO, "%s: IP header from echo reply truncated\n", PROG_NAME);
		exit_clean(EXIT_FAILURE);
	}
	ip_hdr = (struct ip *)reply->recv_buff;
	ip_icmp_offset = ip_hdr->ip_hl << 2;
	if (ip_hdr->ip_p != IPPROTO_ICMP || (size_t)reply->read_bytes < ip_icmp_offset + sizeof(struct icmp)) {
		dprintf(STDERR_FILENO, "%s: ICMP header from echo reply truncated\n", PROG_NAME);
		exit_clean(EXIT_FAILURE);
	}
	reply->icmp_hdr.icmp4 = (struct icmp *)(reply->recv_buff + ip_icmp_offset);
}

void	read_ping_6(struct s_reply *reply) {
	if ((size_t)reply->read_bytes < sizeof(struct icmp6_hdr)) {
		dprintf(STDERR_FILENO, "%s: ICMPv6 header from echo reply truncated\n", PROG_NAME);
		exit_clean(EXIT_FAILURE);
	}
	reply->icmp_hdr.icmp6 = (struct icmp6_hdr *)(reply->recv_buff);
}

void	ping_loop(int family)
{
	union u_ping_pkt	ping_pkt;
	struct timeval		tv_seq_start;
	struct s_reply 		reply;
	bool			send_ping = true;

	if (family == AF_INET)
		printf("PING %s (%s) %zu(%d) bytes of data.\n", g_ping->dest.hostname, g_ping->dest.ip, PING_PKT_DATA_SIZE + sizeof(struct ip), PING_PKT_SIZE);
	else
		printf("PING %s(%s (%s)) %zu data bytes\n", g_ping->dest.hostname, g_ping->dest.reverse_dns, g_ping->dest.ip, sizeof(struct icmp6_hdr) + PING_PKT_DATA_SIZE);

	g_ping->sock_fd = init_socket(family);
	if (gettimeofday((struct timeval *)&g_ping->stat.tv_ping_start, NULL) == -1)
		PERROR("gettimeofday");

	while (true) {
		if (send_ping) {
			if (gettimeofday(&tv_seq_start, NULL) == -1)
				PERROR("gettimeofday");
			if (family == AF_INET)
				fill_ping_pkt_4(&ping_pkt.pkt4, tv_seq_start.tv_sec);
			else
				fill_ping_pkt_6(&ping_pkt.pkt6, tv_seq_start.tv_sec);
			if (g_ping->options.interval > 0) {
				g_ping->wait_alarm = true;
				alarm(g_ping->options.interval);
			}
			if (family == AF_INET) {
				if (sendto(g_ping->sock_fd, &ping_pkt.pkt4, sizeof(ping_pkt.pkt4), 0, (struct sockaddr*)&g_ping->dest.sa_in.ip4, sizeof(g_ping->dest.sa_in.ip4)) == -1)
					PERROR("sendto");
			}
			else {
				fill_send_msg(&reply, &ping_pkt.pkt6);
				if (sendmsg(g_ping->sock_fd, &reply.msg, 0) == -1)
					PERROR("sendmsg");
			}
		}

		fill_recv_msg(&reply);
		reply.read_bytes = recvmsg(g_ping->sock_fd, &reply.msg, 0);
		if (reply.read_bytes == 0) {
			dprintf(STDERR_FILENO, "%s: socket closed\n", PROG_NAME);
			exit_clean(EXIT_FAILURE);
		}
		if (reply.read_bytes == -1) {
			if (errno == EINTR || errno == EAGAIN) {
				send_ping = true;
				if (g_ping->options.set[e_option_verbose])
					dprintf(STDERR_FILENO, "%s: Request timed out for icmp_seq=%hu\n", PROG_NAME, g_ping->stat.icmp_send);
			}
			else
				PERROR("recvmsg");
		}
		else {
			uint16_t rcv_icmp_seq;
			if (family == AF_INET) {
				read_ping_4(&reply);
				rcv_icmp_seq = BSWAP16(reply.icmp_hdr.icmp4->icmp_seq);
				if (rcv_icmp_seq == g_ping->stat.icmp_send || reply.icmp_hdr.icmp4->icmp_type != ICMP_ECHOREPLY) {
					send_ping = inspect_and_print_ping_4(&reply, &tv_seq_start);
					if (!send_ping)
						continue;
				}
				else {
					if (g_ping->options.set[e_option_verbose])
						dprintf(STDERR_FILENO, "%s: received icmp_seq=%hu later\n", PROG_NAME, rcv_icmp_seq);
					send_ping = false;
					continue ;
				}
			}
			else {
				read_ping_6(&reply);
				rcv_icmp_seq = BSWAP16(reply.icmp_hdr.icmp6->icmp6_seq);
				if (rcv_icmp_seq == g_ping->stat.icmp_send || reply.icmp_hdr.icmp6->icmp6_type != ICMP6_ECHO_REPLY) {
					send_ping = inspect_and_print_ping_6(&reply, &tv_seq_start);
					if (!send_ping)
						continue;
				}
				else {
					if (g_ping->options.set[e_option_verbose])
						dprintf(STDERR_FILENO, "%s: received icmp_seq=%hu later\n", PROG_NAME, rcv_icmp_seq);
					send_ping = false;
					continue ;
				}
			}
		}
		if (gettimeofday((struct timeval *)&g_ping->stat.tv_ping_end, NULL) == -1)
			PERROR("gettimeofday");
		if (g_ping->options.set[e_option_count] && g_ping->options.count == g_ping->stat.icmp_send)
			signal_handler_int(0);
		while (g_ping->options.interval > 0 && g_ping->wait_alarm) ;
	}
}
