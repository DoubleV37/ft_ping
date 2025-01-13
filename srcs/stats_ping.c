#include "../ft_ping.h"

float	calc_stats(ping_pckt *pings, int *received, float *min_time, float *max_time) {
	float avg_time = 0;
	float diff_time = 0;
	float avg = 0;

	while (pings) {
		if (pings->recv_time.tv_sec != 0) {
			diff_time = time_diff(pings->sent_time, pings->recv_time);
			if (*min_time == 0 || diff_time < *min_time)
				*min_time = diff_time;
			if (diff_time > *max_time)
				*max_time = diff_time;
			avg += diff_time;
			*received += 1;
		}
		pings = pings->next;
	}

	if (*received > 0) {
		avg_time = avg / *received;
	}
	return avg_time;
}

float calc_variance(ping_pckt *pings, int *received, float *avg_time) {
	float stddev = 0;
	float diff_time = 0;
	float variance = 0;

	while (pings) {
		if (pings->recv_time.tv_sec != 0) {
			diff_time = time_diff(pings->sent_time, pings->recv_time);
			variance += (diff_time - *avg_time) * (diff_time - *avg_time);
		}
		pings = pings->next;
	}

	if (*received > 0) {
		stddev = ft_sqrt(variance / *received);
	}
	return stddev;
}

void print_stats(int sent, ping_pckt *pings) {
	int received = 0;
	float min_time = 0;
	float max_time = 0;
	float avg_time = 0;
	float stddev = 0;
	int percent_loss = 0;

	avg_time = calc_stats(pings, &received, &min_time, &max_time);
	stddev = calc_variance(pings, &received, &avg_time);

	if (sent != 0) {
		percent_loss = (sent - received) * 100 / sent;
	}
	printf("%d packets transmitted, %d packets received, %d%% packet loss\n", sent, received, percent_loss);
	printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n", min_time, avg_time, max_time, stddev);
}
