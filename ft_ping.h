#ifndef FT_PING_H
# define FT_PING_H

# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <sys/time.h>
# include <netdb.h>
# include <netinet/in.h>
# include <netinet/ip.h>
# include <netinet/ip_icmp.h>
# include <net/if.h>
# include <arpa/inet.h>
# include <time.h>
# include <signal.h>
# include <ifaddrs.h>
# include <stdbool.h>
# include <math.h>

extern bool g_run;

typedef struct ping_pckt{
	int seq;
	struct timeval sent_time;
	struct timeval recv_time;
	struct ping_pckt *next;
} ping_pckt;

int	cmd_help(void);
int	cmd_version(void);

ping_pckt* add_ping(ping_pckt *head, int seq_num);
ping_pckt* find_ping(ping_pckt *head, int seq_num);
void	free_ping(ping_pckt *head);
double time_diff(struct timeval start, struct timeval end);
float ft_sqrt(float number);

int	cmd_ping(char *ip_addr_dest);

#endif
