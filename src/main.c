#include "pixelpng.h"
#include "hilbert.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <oping.h>

#define TOTAL_IPS ((int64_t) 256 * 256 * 256 * 256)
#define DIMENSION 65536
#define BIT_DEPTH 1

static int64_t next_ip = 0;
pthread_mutex_t ip_lock;

int get_next_ip(int64_t *ip_number) {
    int has_work = 0;
    pthread_mutex_lock(&ip_lock);
    if (next_ip < TOTAL_IPS) {
        *ip_number = next_ip++;
        has_work = 1;
    }
    pthread_mutex_unlock(&ip_lock);
    return (has_work);
}

static inline void ip_to_str(int64_t ip, char *buf) {
    snprintf(buf, 32, "%d.%d.%d.%d",
             (int)((ip >> 24) & 0xFF),
             (int)((ip >> 16) & 0xFF),
             (int)((ip >> 8)  & 0xFF),
             (int)(ip & 0xFF));
}

struct thread_data {
	pixelPNG* png;
};

void* ping_ip(void* arg) {
	struct thread_data* data = (struct thread_data*) arg;

	int timeout = 1;
	pingobj_t* ping = ping_construct();
	ping_setopt(ping, PING_OPT_TIMEOUT, &timeout);

	int64_t ip_number;
	char ip[32];
	while (get_next_ip(&ip_number)) {
		ip_to_str(ip_number, ip);

		ping_host_add(ping, ip);
		ping_send(ping);

		pingobj_iter_t* iter = ping_iterator_get(ping);

		double latency = 0.0;
		size_t len = sizeof(latency);
		ping_iterator_get_info(iter, PING_INFO_LATENCY, &latency, &len);

		// printf("Pinging %d.%d.%d.%d...\n", data->first_ip_byte, data->second_ip_byte, third, fourth);

		int x, y;
		d2xy(DIMENSION, ip_number, &x, &y);
		if (latency > 0) {
			set_pixel_grayscale(data->png, x, y, 1);
		}
		else {
			set_pixel_grayscale(data->png, x, y, 0);
		}

		ping_host_remove(ping, ip);
	}

	return (NULL);
}

void* print_status(void* arg) {
	while(1) {
		double percentage = (double) next_ip / (double)((int64_t) DIMENSION * DIMENSION);
		printf("\rProgress: %lf%% (%ld/%ld)", percentage, next_ip, (int64_t)DIMENSION * DIMENSION);
		usleep(100000); // 0.1 seconds
	}

	return (arg);
}

int main() {
	pthread_t threads[256 + 1];
	pthread_mutex_init(&ip_lock, NULL);
	pixelPNG* pingmap = initialize_png(DIMENSION, DIMENSION, BIT_DEPTH, GRAYSCALE, 0, 0, 0);

	struct thread_data* data = malloc(sizeof(struct thread_data));
	data->png = pingmap;

	pthread_create(&threads[256], NULL, print_status, NULL);
	for (int i = 0; i < 256; i++) {
		pthread_create(&threads[i], NULL, ping_ip, (void*)data);
	}

	for (int i = 0; i < 256; i++) {
		pthread_join(threads[i], NULL);
	}
	pthread_cancel(threads[256]);
	pthread_join(threads[256], NULL);

	free(data);
	generate_png(pingmap, "pingmap.png");
}
