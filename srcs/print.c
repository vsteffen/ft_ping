#include "ft_ping.h"

void	print_statistics() {
	float	rate;
	double	total_time_elapsed;
	double	rtt_mean;
	double	rtt_mdev;

	printf("\n--- %s ping statistics ---\n", g_ping->dest.hostname);
	rate = ABS((float)(g_ping->stat.icmp_rcv / (float)g_ping->stat.icmp_send) * 100.0 - 100.0);
	if (g_ping->stat.icmp_send == 1)
		total_time_elapsed = 0.0;
	else
		total_time_elapsed = (double)(g_ping->stat.tv_ping_end.tv_sec - g_ping->stat.tv_ping_start.tv_sec) * 1000.0 + (double)(g_ping->stat.tv_ping_end.tv_usec - g_ping->stat.tv_ping_start.tv_usec) / 1000.0;
	if (g_ping->stat.icmp_error > 0)
		printf("%hu packets transmitted, %hu received, +%hu errors, %.0lf%% packet loss, time %.0lfms\n", g_ping->stat.icmp_send, g_ping->stat.icmp_rcv, g_ping->stat.icmp_error, rate, total_time_elapsed);
	else
		printf("%hu packets transmitted, %hu received, %.0lf%% packet loss, time %.0lfms\n", g_ping->stat.icmp_send, g_ping->stat.icmp_rcv, rate, total_time_elapsed);
	if (g_ping->stat.icmp_rcv == g_ping->stat.icmp_send) {
		rtt_mean = g_ping->stat.rtt_sum / g_ping->stat.icmp_rcv;
		rtt_mdev = ft_sqrt_newton(g_ping->stat.rtt_square_sum / g_ping->stat.icmp_rcv - ft_pow(rtt_mean, 2));
		printf("rtt min/avg/max/mdev = %.3lf/%.3lf/%.3lf/%.3lf ms\n", g_ping->stat.rtt_min, rtt_mean, g_ping->stat.rtt_max, rtt_mdev);
	}
}

void	update_stats(double tv_seq_diff) {
	if (g_ping->stat.icmp_rcv == 1)
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

void	print_ping_gen(struct timeval *tv_seq_start, ssize_t read_icmp_bytes, uint16_t rcv_icmp_seq) {
	struct timeval tv_seq_end;

	g_ping->stat.icmp_rcv++;
	if (gettimeofday(&tv_seq_end, NULL) == -1)
		PERROR("gettimeofday");
	double tv_seq_diff = (double)(tv_seq_end.tv_sec - tv_seq_start->tv_sec) * 1000.0 + (double)(tv_seq_end.tv_usec - tv_seq_start->tv_usec) / 1000.0;
	printf("%zd bytes from %s (%s): icmp_seq=%hu ttl=%d time=%.2lf ms\n", read_icmp_bytes, g_ping->dest.reverse_dns, g_ping->dest.ip, rcv_icmp_seq, g_ping->ttl, tv_seq_diff);
	update_stats(tv_seq_diff);
}

void	print_ping_4(struct s_reply *reply, struct timeval *tv_seq_start) {
	uint16_t	rcv_icmp_seq;
	const char	*rcv_code_str;

	rcv_icmp_seq = BSWAP16(reply->icmp_hdr.icmp4->icmp_seq);
	if (reply->icmp_hdr.icmp4->icmp_type != ICMP_ECHOREPLY) {
		g_ping->stat.icmp_error++;
		rcv_code_str = get_error_code_str_4(reply->icmp_hdr.icmp4->icmp_type, reply->icmp_hdr.icmp4->icmp_code);
		if (rcv_code_str)
			printf("From %s (%s): icmp_seq=%hu : %s (%s)\n", g_ping->dest.reverse_dns, g_ping->dest.ip, rcv_icmp_seq, get_error_type_str_4(reply->icmp_hdr.icmp4->icmp_type), rcv_code_str);
		else
			printf("From %s (%s): icmp_seq=%hu : %s (code=%hhu)\n", g_ping->dest.reverse_dns, g_ping->dest.ip, rcv_icmp_seq, get_error_type_str_4(reply->icmp_hdr.icmp4->icmp_type), reply->icmp_hdr.icmp4->icmp_code);
	}
	else
		print_ping_gen(tv_seq_start, reply->read_bytes - sizeof(struct ip), rcv_icmp_seq);
}
