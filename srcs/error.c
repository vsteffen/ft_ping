#include "ft_ping.h"

const char	*get_error_type_str_4(uint8_t type) {
	static const char *type_array[] = {
		"reserved",
		"reserved",
		"reserved",
		"dest unreachable",
		"packet lost, slow down",
		"redirect (change route)",
		"reserved",
		"reserved",
		"reserved",
		"router advertisement",
		"router solicitation",
		"time exceeded",
		"ip header bad",
		"timestamp request",
		"timestamp reply",
		"information request",
		"information reply",
		"address mask request",
		"address mask reply"
	};

	if (type >= sizeof(type_array))
		return ("reserved");
	return (type_array[type]);
}

const char	*get_error_code_str_4(uint8_t type, uint8_t code) {
	static const char *unreach[] = {
		"bad net",
		"bad host",
		"bad protocol",
		"bad port",
		"IP_DF caused drop",
		"src route failed",
		"unknown net",
		"unknown host",
		"src host isolated",
		"net denied",
		"host denied",
		"bad tos for net",
		"bad tos for host",
		"admin prohib",
		"host prec vio.",
		"prec cutoff"
	};
	static const char *redirect[] = {
		"for network",
		"for host",
		"for tos and net",
		"for tos and host"
	};
	static const char *timexceed[] = {
		"ttl==0 in transit",
		"ttl==0 in reass"
	};

	if (type == ICMP_UNREACH && code >= sizeof(unreach))
		return (unreach[code]);
	if (type == ICMP_REDIRECT && code >= sizeof(redirect))
		return (redirect[code]);
	if (type == ICMP_TIMXCEED && code >= sizeof(timexceed))
		return (timexceed[code]);
	if (type == ICMP_PARAMPROB && code == 1)
		return ("req. opt. absent");

	return (NULL);
}
