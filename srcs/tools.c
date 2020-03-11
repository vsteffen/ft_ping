#include "ft_ping.h"

double	ft_sqrt_newton(const double nb)
{
	const double accuracy = SQRT_NEWTON_ACCURACY;
	double	lower;
	double	upper;
	double	sqrt;

	if (nb < 0)
		return (-NAN);
	if (nb == 0)
		return (0.0);
	if (nb < 1) {
		lower = nb;
		upper = 1;
	}
	else {
		lower = 1;
		upper = nb;
	}
	while ((upper - lower) > accuracy) {
		sqrt = (lower + upper) / 2;
		if (sqrt * sqrt > nb)
			upper = sqrt;
		else
			lower = sqrt;
	}
	return ((lower + upper) / 2);
}

double	ft_pow(double nb, uint8_t power)
{
	double		result;
	uint8_t		count;

	count = 0;
	result = 1;
	while (count < power)
	{
		result *= nb;
		count++;
	}
	return (result);
}
