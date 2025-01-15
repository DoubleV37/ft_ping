#include "../ft_ping.h"

char *clean_argv(char *argv)
{
	int i;
	int cpt;
	char *argvclean;

	argvclean = NULL;

	i = 0;
	cpt = 0;
	while (argv[i])
	{
		if (argv[i] == '-')
			cpt++;
		i++;
	}
	argvclean = strdup(argv + cpt);
	return (argvclean);
}

int parsing(int argc, char **argv)
{
	int i = 1;
	char *cmd;

	if (argc == 1)
	{
		printf("ft_ping: missing host operand\n");
		printf("Try 'ft_ping --help' for more information.\n");
		return (1);
	}
	ping ping;
	ping.pings = NULL;
	ping.params.seq = 0;
	ping.socks.recv = -1;
	ping.socks.send = -1;
	ping.params.ttl = 64;
	ping.params.verbose = false;
	ping.params.ip_addr_src = NULL;
	ping.params.raw_dest = NULL;
	ping.params.ip_addr_dest[0] = '\0';
	while (i < argc)
	{
		if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0)
		{
			ping.params.verbose = true;
			printf("verbose\n");
		}
		else if (strcmp(argv[i], "--ttl") == 0)
		{
			if (i + 1 < argc)
			{
				i++;
				ping.params.ttl = atoi(argv[i]);
			}
			else
			{
				printf("ft_ping: option \'--ttl\' requires an argument\n");
				printf("Try 'ft_ping --help' for more information.\n");
				return (1);
			}
		}
		else if (argv[i][0] != '-')
		{
			ping.params.raw_dest = strdup(argv[i]);
			i++;
			continue;
		}
		else if (strcmp(argv[i], "-?") == 0 || strcmp(argv[i], "--help") == 0)
		{
			//free ping->params.ip_addr_dest
			return (cmd_help());
		}
		else if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0)
		{
			//free ping->params.ip_addr_dest
			return (cmd_version());
		}
		else
		{
			cmd = clean_argv(argv[i]);
			printf("ft_ping: invalid option -- '%s'\n", cmd);
			free(cmd);
			return (1);
		}
		i++;
	}
	if (ping.params.raw_dest)
	{
		ping.params.ip_addr_src = get_source_ip();
		get_ip_with_hostname(ping.params.raw_dest, ping.params.ip_addr_dest);
		if (cmd_ping(&ping))
		{
			return (1);
		}
	}
	return (0);
}
