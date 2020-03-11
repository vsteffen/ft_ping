#include "ft_ping.h"

void	signal_handler_alrm(__attribute__((unused))int sig) {
	g_ping->wait_alarm = false;
}

void	signal_handler_int(__attribute__((unused))int sig) {
	print_statistics();
	exit_clean(EXIT_SUCCESS);
}
