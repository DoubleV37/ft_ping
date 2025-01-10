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

long time_diff(struct timeval start, struct timeval end) {
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;

    long total_ms = (seconds * 1000) + (microseconds / 1000);

    return total_ms;
}
