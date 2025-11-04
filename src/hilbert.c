#include "hilbert.h"

// ChatGPT my GOAT
// Rotate/flip a quadrant appropriately
void rot(int n, int *x, int *y, int64_t rx, int64_t ry) {
	if (ry == 0) {
		if (rx == 1) {
			*x = n - 1 - *x;
			*y = n - 1 - *y;
		}
		// Swap x and y
		int t = *x;
		*x = *y;
		*y = t;
	}
}

// Convert Hilbert distance (d) to (x,y)
void d2xy(int n, int64_t d, int *x, int *y) {
	int rx, ry, s, t = d;
	*x = *y = 0;
	for (s = 1; s < n; s *= 2) {
		rx = (t / 2) & 1;
		ry = (t ^ rx) & 1;
		rot(s, x, y, rx, ry);
		*x += s * rx;
		*y += s * ry;
		t /= 4;
	}
	*y = n - 1 - *y;
}
