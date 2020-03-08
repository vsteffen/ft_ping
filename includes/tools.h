#ifndef TOOLS_H
# define TOOLS_H

# define STR_IMPL_(x) #x
# define STR(x) STR_IMPL_(x)

# define PERROR_STR(x) __FILE__ ":" STR(__LINE__) ": " x
# define PERROR(x) { perror(PERROR_STR(x)) ; exit_clean(EXIT_FAILURE); }

#endif
