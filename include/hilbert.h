#ifndef HILBERT_H
# define HILBERT_H

#include <stdint.h>

void rot(int n, int *x, int *y, int64_t rx, int64_t ry);
void d2xy(int n, int64_t d, int *x, int *y);

#endif
