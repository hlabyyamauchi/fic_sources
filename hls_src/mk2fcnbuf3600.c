#include <stdio.h>
#include "ap_cint.h"

#define ISIZE 3600
#define OSIZE 360

#define IPSIZE 45
#define OPSIZE 45

#define IDEPTH (ISIZE/IPSIZE)
#define ODEPTH (OSIZE/OPSIZE)

#define ALL_WB (OSIZE*ISIZE+OSIZE)
#define MBD 4
#define MAX_PKT_SIZE (360/8)
//typedef ap_fixed<16,4> fix;

void buf_fcn_4_384 (
	uint169 sw1in[MAX_PKT_SIZE*(MBD-1)],
	uint169 buf1[MAX_PKT_SIZE*(MBD-1)])
{
#pragma HLS INTERFACE axis port=sw1in
#pragma HLS INTERFACE axis port=buf1

int i;
uint169 tmp[MAX_PKT_SIZE*(MBD-1)];

for(i=0; i<MAX_PKT_SIZE*(MBD-1); i++) {
//	#pragma HLS PIPELINE
	tmp[i] = sw1in[i];
}

for(i=0; i<MAX_PKT_SIZE*(MBD-1); i++) {
//	#pragma HLS PIPELINE
	buf1[i] = tmp[i];
}

return;
}


