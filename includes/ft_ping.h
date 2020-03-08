#ifndef FT_PING_H
# define FT_PING_H

# include "libft.h"

# include "tools.h"

# include <errno.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <netinet/ip_icmp.h>
# include <signal.h>

# define PROG_NAME	"ft_ping"
# define USAGE		"Usage: %s destination\n"

# define DEFAULT_SLEEP_TIME 1

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

typedef struct	s_ping {
	int		sock_fd;
	uint8_t		ttl;
	t_ip_dest	dest;
}		t_ping;

#endif
