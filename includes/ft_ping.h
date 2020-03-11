#ifndef FT_PING_H
# define FT_PING_H

# include "libft.h"

# include "tools.h"

# include <errno.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <netinet/ip_icmp.h>
# include <signal.h>
# include <sys/time.h>

# define PROG_NAME	"ft_ping"
# define USAGE		"Usage: %s destination\n"

# define DEFAULT_SLEEP_TIME	1
# define DEFAULT_TIMEOUT	4000
# define DEFAULT_INTERVAL	1

# define ICMP_ECHO_CODE	0
# define ICMP_ECHO_ID	0

# define TTL	64

# define MAX_INET_ADDRSTRLEN (INET6_ADDRSTRLEN > INET_ADDRSTRLEN ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN)

typedef enum {false, true} bool;

typedef struct	s_ping_pkt4 {
	struct icmphdr	icmp;
}		t_ping_pkt4;

typedef union	u_dest_sockaddr_in {
	struct sockaddr_in	ip4;
	struct sockaddr_in6	ip6;
}		t_sa_in;

typedef struct	s_ip_dest {
	char	*hostname;
	char	*reverse_dns;
	int	family;
	t_sa_in	sa_in;
	char	ip[MAX_INET_ADDRSTRLEN];
}		t_ip_dest;

typedef struct	s_ping_stat {
	size_t		icmp_seq_send;
	size_t		icmp_seq_recv;
	struct timeval	tv_ping_start;
	struct timeval	tv_ping_end;
	double		rtt_square_sum;
	double		rtt_sum;
	double		rtt_min;
	double		rtt_max;
}		t_ping_stat;

typedef struct	s_ping {
	int		sock_fd;
	uint8_t		ttl;
	bool		wait_alarm;
	size_t		interval;
	struct timeval	tv_timeout;
	t_ping_stat	stat;
	t_ip_dest	dest;
}		t_ping;


extern volatile struct s_ping	*g_ping;

void	exit_clean(int exit_status);

void	print_statistics();

// Signals
void	signal_handler_alrm(__attribute__((unused))int sig);
void	signal_handler_int(__attribute__((unused))int sig);


#endif
