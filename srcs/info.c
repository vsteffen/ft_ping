#include "ft_ping.h"

bool	is_info_packet_4(uint8_t type) {
	(void)type;
	return (false);
}

bool	is_info_packet_6(uint8_t type) {
	if (type == ND_ROUTER_SOLICIT)
		return (true);
	if (type == ND_ROUTER_ADVERT)
		return (true);
	if (type == ND_NEIGHBOR_SOLICIT)
		return (true);
	if (type == ND_NEIGHBOR_ADVERT)
		return (true);
	if (type == ND_REDIRECT)
		return (true);
	return (false);
}