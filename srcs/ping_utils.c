#include "../ft_ping.h"

ping_pckt* add_ping(ping_pckt *head, int seq_num) {
	ping_pckt *new_node = (ping_pckt *)malloc(sizeof(ping_pckt));
	if (!new_node)
		return NULL;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	new_node->seq = seq_num;
	new_node->sent_time = tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	new_node->recv_time = tv;
	new_node->next = head;
	return new_node;
}

ping_pckt* find_ping(ping_pckt *head, int seq_num) {
	ping_pckt *current = head;
	while (current) {
		if (current->seq == seq_num) {
			return current;
		}
		current = current->next;
	}
	return NULL;
}

void free_ping(ping_pckt *head) {
	ping_pckt *current = head;
	ping_pckt *next;
	while (current) {
		next = current->next;
		free(current);
		current = next;
	}
}

double time_diff(struct timeval start, struct timeval end) {
	double seconds;
	double microseconds;
	seconds = (double)(end.tv_sec - start.tv_sec);
	microseconds = (double)(end.tv_usec - start.tv_usec);

	return (seconds * 1000.0) + (microseconds / 1000.0);
}

float ft_sqrt(float number) {
	float x = number;
	float y = 1.0;
	float epsilon = 0.0001;

	while (x - y > epsilon) {
		x = (x + y) / 2;
		y = number / x;
	}
	return x;
}

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
