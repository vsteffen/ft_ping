#include "ft_ping.h"

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

void	print_ping_gen(struct timeval *tv_seq_start) {
	struct timeval tv_seq_end;

	g_ping->stat.icmp_seq_recv++;
	if (gettimeofday(&tv_seq_end, NULL) == -1)
		PERROR("gettimeofday");
	double tv_seq_diff = (double)(tv_seq_end.tv_sec - tv_seq_start->tv_sec) * 1000.0 + (double)(tv_seq_end.tv_usec - tv_seq_start->tv_usec) / 1000.0;
	printf("%d bytes from %s (%s): icmp_seq=%zu ttl=%d time=%.2lf ms\n", 64, g_ping->dest.reverse_dns, g_ping->dest.ip, g_ping->stat.icmp_seq_send, g_ping->ttl, tv_seq_diff);
	update_stats(tv_seq_diff);
}

void	print_ping_4(struct s_reply *reply, struct timeval *tv_seq_start) {
	if (reply->icmp_hdr.icmp4->icmp_type != ICMP_ECHOREPLY)
		printf("ECHO REPLY error: ICMP type %hhu and code %hhu\n", reply->icmp_hdr.icmp4->icmp_type, reply->icmp_hdr.icmp4->icmp_code);
	else
		print_ping_gen(tv_seq_start);
}
