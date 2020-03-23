#include "ft_ping.h"

const char	*get_error_type_str_4(uint8_t type) {
	static const char *type_array[] = {
		"echo reply",
		"reserved",
		"reserved",
		"dest unreachable",
		"packet lost, slow down",
		"redirect (change route)",
		"reserved",
		"reserved",
		"echo request",
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
	if (type >= COUNT_OF(type_array))
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

	if (type == ICMP_UNREACH && code < COUNT_OF(unreach))
		return (unreach[code]);
	if (type == ICMP_REDIRECT && code < COUNT_OF(redirect))
		return (redirect[code]);
	if (type == ICMP_TIMXCEED && code < COUNT_OF(timexceed))
		return (timexceed[code]);
	if (type == ICMP_PARAMPROB && code == 1)
		return ("req. opt. absent");

	return (NULL);
}

const char	*get_error_type_str_6(uint8_t type) {
	static const char *type_array1[] = {
		"Reserved",
		"destination unreachable",
		"packet too big",
		"time exceeded",
		"parameter problem"
	};
	static const char *type_array2[] = {
		"Reserved for expansion of ICMPv6 error messages",
		"Echo Request",
		"Echo Reply",
		"Multicast Listener Query",
		"Multicast Listener Report",
		"Multicast Listener Done",
		"Router Solicitation",
		"Router Advertisement",
		"Neighbor Solicitation",
		"Neighbor Advertisement",
		"Redirect Message",
		"Router Renumbering",
		"ICMP Node Information Query",
		"ICMP Node Information Response",
		"Inverse Neighbor Discovery Solicitation Message",
		"Inverse Neighbor Discovery Advertisement Message",
		"Version 2 Multicast Listener Report",
		"Home Agent Address Discovery Request Message",
		"Home Agent Address Discovery Reply Message",
		"Mobile Prefix Solicitation",
		"Mobile Prefix Advertisement",
		"Certification Path Solicitation Message",
		"Certification Path Advertisement Message",
		"ICMP messages utilized by experimental mobility protocols such as Seamoby",
		"Multicast Router Advertisement",
		"Multicast Router Solicitation",
		"Multicast Router Termination",
		"FMIPv6 Messages",
		"RPL Control Message",
		"ILNPv6 Locator Update Message",
		"Duplicate Address Request",
		"Duplicate Address Confirmation",
		"MPL Control Message",
		"Extended Echo Request",
		"Extended Echo Reply"
	};

	if (type < 5)
		return (type_array1[type]);
	if (type < 100)
		return ("Unused");
	if (type == 100 || type == 101 || type == 200 || type == 201)
		return ("Private experimentation");
	if (type < 127)
		return ("Unused");
	if (type < 162)
		return (type_array2[type - 127]);
	if (type == 255)
		return ("Reserved for expansion of ICMPv6 informational messages");
	return ("Unused");
}

const char	*get_error_code_str_6(uint8_t type, uint8_t code) {
	static const char *unreach[] = {
		"no route to destination",
		"communication with destination / administratively prohibited",
		"beyond scope of source address",
		"address unreachable",
		"bad port"
	};
	static const char *timexceed[] = {
		"Hop Limit == 0 in transit",
		"Reassembly time out"
	};
	static const char *paramprob[] = {
		"erroneous header field",
		"unrecognized Next Header",
		"unrecognized IPv6 option"
	};

	if (type == ICMP6_DST_UNREACH && code < COUNT_OF(unreach))
		return (unreach[code]);
	if (type == ICMP6_TIME_EXCEEDED && code < COUNT_OF(timexceed))
		return (timexceed[code]);
	if (type == ICMP6_PARAM_PROB && code < COUNT_OF(paramprob))
		return (paramprob[code]);
	return (NULL);
}