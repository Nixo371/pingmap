#include "pixelpng.h"
#include "hilbert.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <oping.h>

#define DIMENSION 65536
#define BIT_DEPTH 1

#define TOTAL_IPS ((int64_t) 256 * 256 * 256 * 256)
#define BATCH_COUNT 16
#define THREAD_COUNT 1024

static int64_t next_ip = 0;
pthread_mutex_t ip_lock;
pthread_mutex_t png_lock;

int get_next_ip(int64_t *ip_number) {
    int has_work = 0;
    pthread_mutex_lock(&ip_lock);
    if (next_ip * BATCH_COUNT < TOTAL_IPS) {
        *ip_number = next_ip * BATCH_COUNT;
	next_ip++;
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

int64_t get_ip_number(char address[]) {
	int64_t ip_number = 0;

	int tmp = 0;
	for (int i = 0; address[i] != '\0'; i++) {
		if (address[i] == '.') {
			ip_number = (ip_number << 8) + tmp;
			tmp = 0;
			continue;
		}

		tmp *= 10;
		tmp += address[i] - '0';
	}
	ip_number = (ip_number * 256) + tmp;

	return (ip_number);
}

void* ping_ip(void* arg) {
	struct thread_data* data = (struct thread_data*) arg;

	double timeout = 1.0;
	pingobj_t* ping = ping_construct();
	ping_setopt(ping, PING_OPT_TIMEOUT, &timeout);

	int64_t ip_number;
	char ip[32];
	while (get_next_ip(&ip_number)) {
		for (int i = 0; i < BATCH_COUNT; i++) {
			ip_to_str(ip_number + i, ip);

			ping_host_add(ping, ip);
		}
		ping_send(ping);

		pingobj_iter_t* iter;
		for (iter = ping_iterator_get(ping); iter != NULL; iter = ping_iterator_next(iter)) {
			double latency = 0.0;
			size_t len = sizeof(latency);
			ping_iterator_get_info(iter, PING_INFO_LATENCY, &latency, &len);

			char address[40];
			size_t address_len = sizeof(address);
			ping_iterator_get_info(iter, PING_INFO_ADDRESS, address, &address_len);
			int64_t ip_address = get_ip_number(address);

			// printf("%s - %ld.%ld.%ld.%ld\n", address, ((ip_address >> 24) % 256), ((ip_address >> 16) % 256), ((ip_address >> 8) % 256), ((ip_address) % 256));

			int x, y;
			d2xy(DIMENSION, ip_address, &x, &y);
			if (latency > 0) {
				pthread_mutex_lock(&png_lock);
				set_pixel_grayscale(data->png, x, y, 1);
				pthread_mutex_unlock(&png_lock);
			}
			else {
				pthread_mutex_lock(&png_lock);
				set_pixel_grayscale(data->png, x, y, 0);
				pthread_mutex_unlock(&png_lock);
			}
		}

		for (int i = 0; i < BATCH_COUNT; i++) {
			ip_to_str(ip_number + i, ip);

			ping_host_remove(ping, ip);
		}
	}

	ping_destroy(ping);
	return (NULL);
}

void* print_status(void* arg) {
	while(next_ip * BATCH_COUNT < TOTAL_IPS) {
		// double percentage = (double) next_ip / (double)((int64_t) DIMENSION * DIMENSION);
		// printf("\rProgress: %lf%% (%ld/%ld)", percentage, next_ip, (int64_t)DIMENSION * DIMENSION);
		int64_t ip = next_ip * BATCH_COUNT;
		printf("%d.%d.%d.%d\r", ((int)(ip >> 24) % 256), ((int)(ip >> 16) % 256), ((int)(ip >> 8) % 256), ((int)(ip) % 256));
	}

	return (arg);
}

int main() {
	pthread_t threads[THREAD_COUNT + 1];
	pthread_mutex_init(&ip_lock, NULL);
	pthread_mutex_init(&png_lock, NULL);
	pixelPNG* pingmap = initialize_png(DIMENSION, DIMENSION, BIT_DEPTH, GRAYSCALE, 0, 0, 0);

	struct thread_data* data = malloc(sizeof(struct thread_data));
	data->png = pingmap;

	pthread_create(&threads[THREAD_COUNT], NULL, print_status, NULL);
	for (int i = 0; i < THREAD_COUNT; i++) {
		pthread_create(&threads[i], NULL, ping_ip, (void*)data);
	}

	for (int i = 0; i < THREAD_COUNT; i++) {
		pthread_join(threads[i], NULL);
	}
	pthread_join(threads[THREAD_COUNT], NULL);

	free(data);
	generate_png(pingmap, "pingmap.png");

	free_png(pingmap);
}
