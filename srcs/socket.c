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

void prepare_data(struct sockaddr_in *dest_addr, char *ip_addr_dest, char *packet, size_t packet_size)
{
    struct icmp icmp_hdr;

    // Configure destination address
    memset(dest_addr, 0, sizeof(*dest_addr));
    dest_addr->sin_family = AF_INET;
    dest_addr->sin_addr.s_addr = inet_addr(ip_addr_dest);

    // Prepare ICMP header
    memset(&icmp_hdr, 0, sizeof(icmp_hdr));
    icmp_hdr.icmp_type = ICMP_ECHO;
    icmp_hdr.icmp_code = 0;
    icmp_hdr.icmp_id = htons(getpid() & 0xFFFF);
    icmp_hdr.icmp_seq = htons(1);

    // Clear the entire packet buffer
    memset(packet, 0, packet_size);

    // Copy ICMP header into the packet
    memcpy(packet, &icmp_hdr, sizeof(icmp_hdr));

    // Fill the remaining space in the packet with a pattern or data
    char *data_part = packet + sizeof(icmp_hdr);
    memset(data_part, 0x42, packet_size - sizeof(icmp_hdr)); // Fill with pattern '0x42'

    // Calculate checksum over the entire packet
    struct icmp *icmp_packet = (struct icmp *)packet; // Access as ICMP struct
    icmp_packet->icmp_cksum = 0;                      // Set checksum field to 0 first
    icmp_packet->icmp_cksum = checksum(packet, packet_size); // Compute checksum
}

int send_packet(int sockfd, struct sockaddr_in *dest_addr, char *packet, size_t packet_size)
{
	if (sendto(sockfd, packet, packet_size, 0, (struct sockaddr *)dest_addr, sizeof(*dest_addr)) <= 0)
	{
		perror("Sendto error");
		close(sockfd);
		return 1;
	}
	return 0;
}

int cmd_ping(char *ip_addr_dest) {
    int sockfd;
    char packet[84];  // Standard ping packet size (ICMP header + 56 bytes of data)
    struct sockaddr_in dest_addr;
    char recv_buffer[1500];  // Large enough for a typical MTU
    struct ip *ip_hdr;
    struct icmp *icmp_reply;

    sockfd = create_socket();
    if (sockfd < 0) {
        return 1;
    }
	prepare_data(&dest_addr, ip_addr_dest, packet, sizeof(packet))
	if (send_packet(sockfd, &dest_addr, packet, sizeof(packet)) != 0) {
		return 1;
	}

    while (1) {
        ssize_t recv_len = recvfrom(sockfd, recv_buffer, sizeof(recv_buffer), 0, NULL, NULL);
        if (recv_len <= 0) {
            perror("Recvfrom error or timeout");
            close(sockfd);
            return 1;
        }

        // Navigate past IP header to ICMP packet
        ip_hdr = (struct ip *)recv_buffer;
        int ip_header_len = ip_hdr->ip_hl * 4;
        icmp_reply = (struct icmp *)(recv_buffer + ip_header_len);
        // printf("Réponse reçue: Type %d, Code %d, ID %d, Seq %d\n", icmp_reply->icmp_type, icmp_reply->icmp_code, ntohs(icmp_reply->icmp_id), ntohs(icmp_reply->icmp_seq));
        if (icmp_reply->icmp_type == ICMP_ECHOREPLY && ntohs(icmp_reply->icmp_id) == (getpid() & 0xFFFF)) {
            printf("Ping réussi! Réponse de %s\n", ip_addr_dest);
			send_packet(sockfd, &dest_addr, packet, sizeof(packet));
            // close(sockfd);
            // return 0;
        }
    }

    printf("Aucune réponse valide reçue\n");
    close(sockfd);
    return 1;
}
