#include <stdio.h>
#include <ap_fixed.h>
#define ISIZE 3600
#define OSIZE 360

#define IPSIZE 45
#define OPSIZE 45

#define IDEPTH (ISIZE/IPSIZE)
#define ODEPTH (OSIZE/OPSIZE)

#define ALL_WB (OSIZE*ISIZE+OSIZE)
#define MBD 4
#define MAX_PKT_SIZE (360/8)
typedef ap_fixed<16,4> fix;

void fcninputopt(fix input[ISIZE], fix wb[ALL_WB]) {
#pragma HLS INTERFACE axis register both port=input
#pragma HLS INTERFACE axis register both port=wb
	int i;
	for (i = 0; i < ALL_WB; i++) {
		wb[i] = 1;
	}
	for (i = 0; i < ISIZE; i++) {
		input[i] = 1;
	}
	return;
}
