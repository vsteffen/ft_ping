#ifndef FT_PING_H
# define FT_PING_H

# include "libft.h"

# include "tools.h"

# include <errno.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <netinet/ip_icmp.h>
# include <netinet/icmp6.h>
# include <netinet/ip6.h>
# include <signal.h>
# include <sys/time.h>

# define PROG_NAME	"ft_ping"
# define USAGE		"Usage: %s [-v64] [-c count] [-i interval] [-t ttl] [-W timeout] destination\n"

# define DEFAULT_TTL		64
# define DEFAULT_SLEEP_TIME	1
# define DEFAULT_INTERVAL	1

# define PING_ICMP_ECHO_CODE	0

# define PING_PKT_SIZE		84
# define PING_PKT_DATA_SIZE	(PING_PKT_SIZE - sizeof(struct icmp) - sizeof(time_t))

# define MAX_INET_ADDRSTRLEN (INET6_ADDRSTRLEN > INET_ADDRSTRLEN ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN)

typedef enum {false, true} bool;

typedef enum {e_ip4, e_ip6} e_version;

typedef enum {
	e_option_version,
	e_option_count,
	e_option_interval,
	e_option_ttl,
	e_option_timeout,
	e_option_verbose,
	e_options_count
}	e_options;

# define OPTION_VERSION_4_CHAR	'4'
# define OPTION_VERSION_6_CHAR	'6'
# define OPTION_COUNT_CHAR	'c'
# define OPTION_INTERVAL_CHAR	'i'
# define OPTION_TTL_CHAR	't'
# define OPTION_TIMEOUT_CHAR	'W'
# define OPTION_VERBOSE_CHAR	'v'
# define OPTION_USAGE		'h'

typedef struct __attribute__((packed))	s_ping_pkt4 {
	struct icmp	icmp;
	time_t		timestamp;
	char		data[PING_PKT_DATA_SIZE];
}					t_ping_pkt4;

typedef struct __attribute__((packed))	s_ping_pkt6 {
	struct icmp6_hdr	icmp;
	time_t			timestamp;
	char			data[PING_PKT_DATA_SIZE];
}					t_ping_pkt6;

typedef union	u_dest_sockaddr_in {
	struct sockaddr_in	ip4;
	struct sockaddr_in6	ip6;
}		t_sa_in;

union	u_icmp_hdr {
	struct icmp		*icmp4;
	struct icmp6_hdr	*icmp6;
};

union	u_ping_pkt {
	struct s_ping_pkt4	pkt4;
	struct s_ping_pkt6	pkt6;
};

typedef struct	s_reply {
	char			recv_buff[PING_PKT_SIZE];
	struct msghdr		msg;
	struct iovec		iov;
	ssize_t			read_bytes;
	union u_icmp_hdr	icmp_hdr;
	union	{
		char		buff[CMSG_SPACE(sizeof(int))];
		struct cmsghdr	align;
	} ctrl;
}		t_reply;

typedef struct	s_ip_dest {
	char	*hostname;
	char	*reverse_dns;
	int	family;
	t_sa_in	sa_in;
	char	ip[MAX_INET_ADDRSTRLEN];
}		t_ip_dest;

typedef struct	s_ping_stat {
	uint16_t	icmp_send;
	uint16_t	icmp_rcv;
	uint16_t	icmp_error;
	struct timeval	tv_ping_start;
	struct timeval	tv_ping_end;
	double		rtt_square_sum;
	double		rtt_sum;
	double		rtt_min;
	double		rtt_max;
}		t_ping_stat;

typedef struct	s_ping_options {
	bool		set[e_options_count];
	e_version	version;
	size_t		count;
	size_t		interval;
	int		ttl;
	struct timeval	timeout;
}		t_ping_options;

typedef struct	s_ping {
	int		sock_fd;
	bool		wait_alarm;
	t_ping_stat	stat;
	t_ip_dest	dest;
	t_ping_options	options;
}		t_ping;


extern volatile struct s_ping	*g_ping;

void	exit_clean(int exit_status);
bool	parse_options(int ac, char **av, struct s_ping *ping);

void	ping_loop(int family);

// Signals
void	signal_handler_alrm(__attribute__((unused))int sig);
void	signal_handler_int(__attribute__((unused))int sig);

// Error
const char	*get_error_type_str_4(uint8_t type);
const char	*get_error_code_str_4(uint8_t type, uint8_t code);
const char	*get_error_type_str_6(uint8_t type);
const char	*get_error_code_str_6(uint8_t type, uint8_t code);

// Info
bool	is_info_packet_4(uint8_t type);
bool	is_info_packet_6(uint8_t type);

// Print
bool	inspect_and_print_ping_4(struct s_reply *reply, struct timeval *tv_seq_start);
bool	inspect_and_print_ping_6(struct s_reply *reply, struct timeval *tv_seq_start);
void	print_statistics();

#endif
