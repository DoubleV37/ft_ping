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

extern bool g_run;

typedef struct ping_pckt{
	int seq;
	struct timeval sent_time;
	struct timeval recv_time;
	struct ping_pckt *next;
} ping_pckt;

typedef struct s_ping_data{
	struct ip ip_hdr;
	struct icmp icmp_hdr;
	char data[56];
} ping_data;

typedef struct s_socks{
	int recv;
	int send;
} socks;

typedef struct s_params{
	uint16_t id;
	char *ip_addr_src;
	char *raw_dest;
	char ip_addr_dest[16];
	int ttl;
	bool verbose;
	int seq;
} params;

typedef struct s_ping{
	params params;
	socks socks;
	ping_pckt *pings;
} ping;


int	cmd_help(void);
int	cmd_version(void);

ping_pckt* add_ping(ping_pckt *head, int seq_num);
ping_pckt* find_ping(ping_pckt *head, int seq_num);
void	free_ping(ping_pckt *head);
double time_diff(struct timeval start, struct timeval end);
float ft_sqrt(float number);
unsigned short checksum(void *b, int len);

void print_stats(int sent, ping_pckt *pings);

int parsing(int argc, char **argv);

int	cmd_ping(ping *ping);
char	*get_source_ip();

#endif
