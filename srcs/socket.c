#include "../ft_ping.h"

unsigned short checksum(void *b, int len) {
	unsigned short *buf = b;
	unsigned int sum = 0;
	unsigned short result;

	for (sum = 0; len > 1; len -= 2) {
		sum += *buf++;
	}

	if (len == 1) {
		sum += *(unsigned char *)buf;
	}

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	result = ~sum;
	return result;
}

int	create_socket(void)
{
	int	sock;
	struct timeval timeout;

	sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
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

int send_ping(int sock, struct sockaddr_in *dest_addr, int sequence) {
    char packet[64];
    struct icmp *header = (struct icmp *)packet;

    header->icmp_type = ICMP_ECHO;
    header->icmp_code = 0;
    header->icmp_id = htons(getpid());
    header->icmp_seq = htons(sequence);
	header->icmp_cksum = 0;

    header->icmp_cksum = checksum((uint16_t *)packet, sizeof(packet));

    if (sendto(sock, packet, sizeof(packet), 0, (struct sockaddr *)dest_addr, sizeof(*dest_addr)) < 0) {
        perror("Send failed");
        return 1;
    }
    return 0;
}

int recv_ping(int sock) {
	char recv_buffer[1500];
	struct ip *ip_hdr;
	struct icmp *icmp_reply;

	ssize_t recv_len = recvfrom(sock, recv_buffer, sizeof(recv_buffer), 0, NULL, NULL);
	if (recv_len <= 0) {
		perror("Recvfrom error or timeout");
		return 1;
	}
	ip_hdr = (struct ip *)recv_buffer;
	int ip_header_len = ip_hdr->ip_hl * 4;
	icmp_reply = (struct icmp *)(recv_buffer + ip_header_len);

	if (icmp_reply->icmp_type == ICMP_ECHOREPLY && ntohs(icmp_reply->icmp_id) == (getpid() & 0xFFFF)) {
		printf("%ld bytes from %s: icmp_seq=%d ttl=%d\n", recv_len - ip_header_len, inet_ntoa(ip_hdr->ip_src), ntohs(icmp_reply->icmp_seq), ip_hdr->ip_ttl);
		return 0;
	}
	return 1;
}

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

int cmd_ping(char *raw_ip_addr_dest) {
	int sockfd;
	struct sockaddr_in dest_addr;
	int sequence;

	char ip_addr_dest[INET_ADDRSTRLEN];
	get_ip_with_hostname(raw_ip_addr_dest, ip_addr_dest);

	sockfd = create_socket();
	if (sockfd < 0) {
		return 1;
	}
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_addr.s_addr = inet_addr(ip_addr_dest);
	printf("PING %s (%s): 56 data bytes\n", raw_ip_addr_dest, ip_addr_dest);
	sequence = 0;
	while (1) {
		if (send_ping(sockfd, &dest_addr, sequence) != 0) {
			close(sockfd);
			return 1;
		}
		recv_ping(sockfd);
		sequence++;
		sleep(1);
	}

	printf("Aucune réponse valide reçue\n");
	close(sockfd);
	return 1;
}
