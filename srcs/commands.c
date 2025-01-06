#include "../ft_ping.h"

int	cmd_help(void)
{
	printf("Usage: ft_ping [OPTION...] HOST ...\n");
	printf("Send ICMP ECHO_REQUEST packets to network hosts.\n");
	printf("\n");
	printf("  -?, --help     give this help list\n");
	printf("  -V, --version  print program version\n");
	printf("  -v  --verbose  verbose output\n");
	return (0);
}

int	cmd_version(void)
{
	printf("ft_ping (take as reference ping from inetutils-2.0)\n");
	printf("Written by Valentin Viovi, 42 Student\n");
	return (0);
}
