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
	return (argvclean = strdup(argv + cpt));
}

int parsing(int argc, char **argv)
{
	int i;
	char *cmd;

	i = 1;
	while (i < argc)
	{
		if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0)
			printf("verbose\n");
		else if (argv[i][0] != '-')
		{
			cmd_ping(argv[i]);
		}
		else if (strcmp(argv[i], "-?") == 0 || strcmp(argv[i], "--help") == 0)
			return (cmd_help());
		else if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0)
			return (cmd_version());
		else
		{
			cmd = clean_argv(argv[i]);
			printf("ft_ping: invalid option -- '%s'\n", cmd);
			free(cmd);
			return (1);
		}
		i++;
	}
	if (argc == 1)
	{
		printf("ft_ping: missing host operand\n");
		printf("Try 'ft_ping --help' for more information.\n");
	}
	return (1);
}

int	main(int argc, char **argv)
{
	if (getgid() != 0)
	{
		printf("ft_ping: You must have root privilege\n");
		return (1);
	}
	if (!parsing(argc, argv))
		return (1);
	return (0);
}
