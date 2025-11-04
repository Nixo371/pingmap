#include "pixelpng.h"
#include "hilbert.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIMENSION 65536
#define BIT_DEPTH 1

char* generate_ip_address(int64_t ip_number) {
	int first = (ip_number >> 24) % 256;
	int second = (ip_number >> 16) % 256;
	int third = (ip_number >> 8) % 256;
	int fourth = (ip_number) % 256;

	char* ip_address = malloc(32 * sizeof(char));
	sprintf(ip_address, "%d.%d.%d.%d", first, second, third, fourth);

	return (ip_address);
}

char* generate_ping_command(int64_t ip_number) {
	char* ip_address = generate_ip_address(ip_number);

	// ping command
	// one packet (-c 1)
	// wait max 1 second (-w 1)
	char* ping_command_base = "ping -c 1 -w 1 ";
	char* ping_command_end = " > /dev/null 2>&1";
	char* ping_command = malloc((strlen(ping_command_base) + strlen(ping_command_end) + 16) * sizeof(char));
	sprintf(ping_command, "%s%s%s", ping_command_base, ip_address, ping_command_end);

	return (ping_command);
}

int main() {
	pixelPNG* pingmap = initialize_png(DIMENSION, DIMENSION, BIT_DEPTH, GRAYSCALE, 0, 0, 0);

	for (int64_t i = 0; i < (int64_t)DIMENSION * DIMENSION; i++) {
		int x, y;
		d2xy(DIMENSION, i, &x, &y);

		char* ping_command = generate_ping_command(i);
		char* ip_address = generate_ip_address(i);

		int ping = system(ping_command);
		if (ping == 0) {
			set_pixel_grayscale(pingmap, x, y, 1);
		}
		else {
			set_pixel_grayscale(pingmap, x, y, 0);
		}

		free(ping_command);
	}

	generate_png(pingmap, "hilbert_test.png");
}
