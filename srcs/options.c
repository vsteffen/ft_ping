#include "ft_ping.h"

static inline bool	check_value_null(char *value, char option_char) {
	if (!value) {
		dprintf(STDERR_FILENO, "%s: option requires an argument -- '%c'\n", PROG_NAME, option_char);
		return (true);
	}
	return (false);
}

static bool	handle_option(char *option, char *value, int *i, struct s_ping *ping) {
	size_t	j = 0;
	int	value_int;

	if (option[j] == '\0') {
		dprintf(STDERR_FILENO, "%s: invalid option -- '%c'\n", PROG_NAME, '-');
		return (false);
	}
	while (option[j] != '\0') {
		if (option[j] == OPTION_VERSION_4_CHAR) {
			if (ping->options.set[e_option_version]) {
				dprintf(STDERR_FILENO, "%s: Only one -4 or -6 option may be specified\n", PROG_NAME);
				return (false);
			}
			ping->options.set[e_option_version] = true;
			ping->options.version = e_ip4;
		}
		else if (option[j] == OPTION_VERSION_6_CHAR) {
			if (ping->options.set[e_option_version]) {
				dprintf(STDERR_FILENO, "%s: Only one -4 or -6 option may be specified\n", PROG_NAME);
				return (false);
			}
			ping->options.set[e_option_version] = true;
			ping->options.version = e_ip6;
		}
		else if (option[j] == OPTION_COUNT_CHAR) {
			if (check_value_null(value, option[j]))
				return (false);
			if ((value_int = ft_atoi(value)) <= 0) {
				dprintf(STDERR_FILENO, "%s: bad number of packets to transmit\n", PROG_NAME);
				return (false);
			}
			ping->options.set[e_option_count] = true;
			ping->options.count = (size_t)value_int;
			(*i)++;
			return (true);
		}
		else if (option[j] == OPTION_INTERVAL_CHAR) {
			if (check_value_null(value, option[j]))
				return (false);
			if ((value_int = ft_atoi(value)) < 0) {
				dprintf(STDERR_FILENO, "%s: bad timing interval\n", PROG_NAME);
				return (false);
			}
			ping->options.set[e_option_interval] = true;
			ping->options.interval = (size_t)value_int;
			(*i)++;
			return (true);
		}
		else if (option[j] == OPTION_TTL_CHAR) {
			if (check_value_null(value, option[j]))
				return (false);
			value_int = ft_atoi(value);
			if (0 > value_int || value_int > 255) {
				dprintf(STDERR_FILENO, "%s: ttl %u out of range\n", PROG_NAME, (uint32_t)value_int);
				return (false);
			}
			ping->options.set[e_option_ttl] = true;
			ping->options.ttl = value_int;
			(*i)++;
			return (true);
		}
		else if (option[j] == OPTION_TIMEOUT_CHAR) {
			if (check_value_null(value, option[j]))
				return (false);
			if ((value_int = ft_atoi(value)) < 0) {
				dprintf(STDERR_FILENO, "%s: bad linger time\n", PROG_NAME);
				return (false);
			}
			ping->options.set[e_option_timeout] = true;
			ping->options.timeout.tv_sec = (size_t)value_int / 1000;
			ping->options.timeout.tv_usec = (size_t)value_int * 1000 % 1000000;
			(*i)++;
			return (true);
		}
		else if (option[j] == OPTION_VERBOSE_CHAR) {
			ping->options.set[e_option_verbose] = true;
		}
		else if (option[j] == OPTION_USAGE) {
			return (false);
		}
		else {
			dprintf(STDERR_FILENO, "%s: invalid option -- '%c'\n", PROG_NAME, option[j]);
			return (false);
		}
		j++;
	}
	return (true);
}

bool		parse_options(int ac, char **av, struct s_ping *ping) {
	int	i = 0;
	bool	set_dest_hostname = false;

	if (ac < 2)
		return (false);
	ft_bzero(&ping->options, sizeof(ping->options));
	while (++i < ac) {
		if (av[i][0] == '-') {
			if (!handle_option(av[i] + 1, av[i + 1], &i, ping))
				return (false);
		}
		else {
			if (set_dest_hostname) {
				dprintf(STDERR_FILENO, "%s: hostname already set\n", PROG_NAME);
				return (false);
			}
			ping->dest.hostname = av[i];
			set_dest_hostname = true;
		}
	}
	if (!set_dest_hostname)
		return (false);
	return (true);
}