#include "../ft_ping.h"

bool g_run = true;

void	handle_sigint(int sig)
{
	(void)sig;
	g_run = false;
}

int	main(int argc, char **argv)
{
	signal(SIGINT, handle_sigint);
	if (getgid() != 0)
	{
		printf("ft_ping: You must have root privilege\n");
		return (1);
	}
	if (!parsing(argc, argv))
		return (1);
	return (0);
}
