#include "../ft_ping.h"

void	get_ip_with_hostname(char *hostname, char final_ip[INET_ADDRSTRLEN]) {
	struct addrinfo hints, *info, *p;
	int gai_result;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_CANONNAME;

	if ((gai_result = getaddrinfo(hostname, "http", &hints, &info)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gai_result));
		exit(1);
	}

	for(p = info; p != NULL; p = p->ai_next) {
		void *addr;
		if (p->ai_family == AF_INET) {
			addr = &((struct sockaddr_in *)p->ai_addr)->sin_addr;
			inet_ntop(p->ai_family, addr, final_ip, INET_ADDRSTRLEN);
			freeaddrinfo(info);
			return;
		}
	}
	freeaddrinfo(info);
}

char	*get_source_ip() {
	char *ip = NULL;
	struct ifaddrs *ifaddr, *ifa;
	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		return (NULL);
	}
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL)
			continue;
		if (ifa->ifa_addr->sa_family == AF_INET && !(ifa->ifa_flags & IFF_LOOPBACK)) {
			struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;
			ip = inet_ntoa(sa->sin_addr);
			freeifaddrs(ifaddr);
			return (ip);
		}
	}
	freeifaddrs(ifaddr);
	return (NULL);
}

int	create_socket_recv(void)
{
	int	sock;
	struct timeval timeout;

	sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sock)
	if (sock < 0)
	{
		perror("socket error");
		return (-1);
	}
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
		perror("setsockopt error");
		close(sock);
		return -1;
	}

	return (sock);
}

int create_socket_send(void) {
	int sock;

	sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if (sock < 0) {
		perror("Socket error");
		return -1;
	}
	int disable = 1;
	if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &disable, sizeof(disable)) < 0) {
		perror("setsockopt error");
		close(sock);
		return -1;
	}
	return sock;
}

int send_ping(ping *ping) {
	struct sockaddr_in dest_addr;
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_addr.s_addr = inet_addr(ping->params.ip_addr_dest);
    size_t packet_size = sizeof(ping_data);
    ping_data *data = (ping_data *)malloc(packet_size);
    if (!data) {
        perror("Memory allocation failed");
        return 1;
    }

    memset(data, 0, packet_size);
    data->ip_hdr.ip_hl = 5;
    data->ip_hdr.ip_v = 4;
    data->ip_hdr.ip_tos = 0;
    data->ip_hdr.ip_len = htons(packet_size);
    data->ip_hdr.ip_id = ping->params.id;
    data->ip_hdr.ip_off = 0;
    data->ip_hdr.ip_ttl = ping->params.ttl;
    data->ip_hdr.ip_p = IPPROTO_ICMP;
    data->ip_hdr.ip_src.s_addr = inet_addr(ping->params.ip_addr_src);
    data->ip_hdr.ip_dst.s_addr = dest_addr.sin_addr.s_addr;
    data->ip_hdr.ip_sum = 0;
    data->ip_hdr.ip_sum = checksum((uint16_t *)&data->ip_hdr, sizeof(data->ip_hdr));

	data->icmp_hdr.type = ICMP_ECHO;
	data->icmp_hdr.code = 0;
	data->icmp_hdr.un.echo.id = htons(ping->params.id);
	data->icmp_hdr.un.echo.sequence = ping->params.seq;
	data->icmp_hdr.checksum = 0;
	data->icmp_hdr.checksum = checksum((uint16_t *)&data->icmp_hdr, sizeof(data->icmp_hdr));

    if (sendto(ping->socks.send, data, packet_size, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("Send failed");
        free(data);
        return 1;
    }
    free(data);
    return 0;
}


int recv_ping(ping *ping) {
	char recv_buffer[4096];
	struct ip *ip_hdr;
	struct icmphdr *icmp_reply;
	struct timeval recv_time;
	long nb_bytes;
	float diff = 0;
	struct sockaddr_in	src_addr;
	socklen_t		addr_len = sizeof(struct sockaddr_in);

	ssize_t recv_len = recvfrom(ping->socks.recv, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr*)&src_addr, &addr_len);
	if (recv_len < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
		perror("Recvfrom timeout");
		return 1;
	}
	if (recv_len <= 0) {
		return 1;
	}
	ip_hdr = (struct ip *)recv_buffer;
	int ip_header_len = ip_hdr->ip_hl * 4;
	icmp_reply = (struct icmphdr *)(recv_buffer + ip_header_len);

	if (ip_hdr->ip_id == ping->params.id) {
		return 2;
	}

	if (icmp_reply->type == ICMP_TIME_EXCEEDED) {
		printf("%ld bytes from %s: Time to live exceeded\n", recv_len - ip_header_len, inet_ntoa(ip_hdr->ip_src));
		if (ping->params.verbose) {
			ip_hdr = (struct ip *)(recv_buffer + ip_header_len + 8);
			printf("IP Hdr Dump\n");
			printf(" ");
			for (int i = 0; i < ip_header_len; i++) {
				printf("%02x", ((unsigned char *)ip_hdr)[i]);
				if (i % 2 == 1)
					printf(" ");
			}
			printf("\n");
			printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src        Dst       Data\n");
			char *src = strdup(inet_ntoa(ip_hdr->ip_src));
			printf(" %1x  %1x  %02x %04x %04x   %1x %04x  %02x  %02x %04x %-15s %-15s\n",
				ip_hdr->ip_v,
				ip_hdr->ip_hl,
				ip_hdr->ip_tos,
				ntohs(ip_hdr->ip_len),
				ntohs(ip_hdr->ip_id),
				(ntohs(ip_hdr->ip_off) >> 13) & 0x07,
				ntohs(ip_hdr->ip_off) & 0x1FFF,
				ip_hdr->ip_ttl,
				ip_hdr->ip_p,
				ntohs(ip_hdr->ip_sum),
				src,
				inet_ntoa(ip_hdr->ip_dst));
			free(src);
			icmp_reply = (struct icmphdr *)(recv_buffer + ip_header_len + 8 + ip_header_len);
			printf("ICMP: type %d, code %d, size %d, id 0x%04x, seq 0x%04x\n", icmp_reply->type, icmp_reply->code, ntohs(ip_hdr->ip_len) - ip_header_len, ntohs(icmp_reply->un.echo.id), ntohs(icmp_reply->un.echo.sequence));
		}
		return 0;
	}
	if (icmp_reply->type == ICMP_ECHOREPLY) {
		ping_pckt *ping_target = find_ping(ping->pings, icmp_reply->un.echo.sequence);
		if (ping_target) {
			gettimeofday(&recv_time, NULL);
			ping_target->recv_time = recv_time;
			diff = time_diff(ping_target->sent_time, ping_target->recv_time);
		}
		nb_bytes = recv_len - ip_header_len;
		printf("%ld bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n", nb_bytes, ping->params.ip_addr_dest, ntohs(icmp_reply->un.echo.sequence), ip_hdr->ip_ttl, diff);
		return 0;
	}
	if (icmp_reply->type == 8) {
		printf("From %s icmp_seq=%d Request\n", ping->params.ip_addr_dest, ntohs(icmp_reply->un.echo.sequence));
		return 0;
	}
	return 1;
}

void print_first_line(ping *ping) {
	if (ping->params.verbose)
		printf("PING %s (%s): 56 data bytes, id 0x%x = %d\n", ping->params.raw_dest, ping->params.ip_addr_dest, ping->params.id, ping->params.id);
	else
		printf("PING %s (%s): 56 data bytes\n", ping->params.raw_dest, ping->params.ip_addr_dest);
}

int cmd_ping(ping *ping) {
	ping->socks.recv = create_socket_recv();
	if (ping->socks.recv < 0) {
		return 1;
	}
	ping->socks.send = create_socket_send();
	if (ping->socks.send < 0) {
		close(ping->socks.recv);
		return 1;
	}
	ping->params.id = getpid() & 0xFFFF;

	print_first_line(ping);
	while (g_run) {
		if (send_ping(ping) != 0) {
			close(ping->socks.recv);
			close(ping->socks.send);
			return 1;
		}
		ping->pings = add_ping(ping->pings, ping->params.seq);
		while (recv_ping(ping) == 2) {
			if (!g_run) {
				break;
			}
		}
		uint16_t seq = ntohs(ping->params.seq);
		seq++;
		ping->params.seq = htons(seq);
		sleep(1);
	}
	printf("--- %s ft_ping statistics ---\n", ping->params.raw_dest);
	print_stats(ntohs(ping->params.seq), ping->pings);
	close(ping->socks.recv);
	close(ping->socks.send);
	free_ping(ping->pings);
	return 1;
}
