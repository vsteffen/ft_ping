#ifndef TOOLS_H
# define TOOLS_H

#include <math.h>

# define STR_IMPL_(x) #x
# define STR(x) STR_IMPL_(x)

# define PERROR_STR(x) __FILE__ ":" STR(__LINE__) ": " x
# define PERROR(x) { perror(PERROR_STR(x)) ; exit_clean(EXIT_FAILURE); }

# define ABS(x)	(x < 0 ? -(x) : x)

# define SQRT_NEWTON_ACCURACY 0.0001

double	ft_sqrt_newton(const double nb);
double	ft_pow(double nb, uint8_t power);

#endif
