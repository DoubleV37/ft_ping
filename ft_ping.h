#ifndef FT_PING_H
# define FT_PING_H

# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <netdb.h>
# include <netinet/in.h>
# include <netinet/ip.h>
# include <netinet/ip_icmp.h>
# include <arpa/inet.h>
# include <time.h>

// int parsing(int argc, char **argv);

int	cmd_help(void);
int	cmd_version(void);

int	cmd_ping(char *ip_addr_dest);

#endif
