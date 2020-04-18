#include <ap_fixed.h>
//#include "ap_cint.h"
#define MBD 2 // number of boards
//conv1 parameter
#define C1_OCH 20
#define C1_OSIZE 24
#define C1_ICH 1
#define C1_ISIZE 28
#define C1_K 5
//conv1 partition parameter
#define CONV1_OUTLOOP 24 // outer loop == C1_OSIZE
#define CONV1_LOOPEXE 12 // ceiling(CONV1_OUTLOOP/MBD)
#define CONV1_OUT_SIZE C1_OCH*C1_ICH*C1_OSIZE*C1_OSIZE
#define CONV1_BUF_SIZE (C1_OCH*C1_ICH*C1_OSIZE*CONV1_LOOPEXE+3)//means CONV1_OUT_SIZE/CONV1_OUTLOOP*CONV1_LOOPEXE   +3 ceiling for pkt
#define CONV1_PKT_SIZE (CONV1_BUF_SIZE/4) //send packetnum

#define P1_OSIZE 12
#define P1_K 2

#define C2_OCH 50
#define C2_OSIZE 8
#define C2_ICH 20
#define C2_ISIZE 12
#define C2_K 5
#define C2_P 10

#define CONV2_OUTLOOP 8
#define CONV2_LOOPEXE 4
#define CONV2_OUT_SIZE C2_OCH*C2_OSIZE*C2_OSIZE
#define CONV2_BUF_SIZE (C2_OCH*C2_OSIZE*CONV2_LOOPEXE+3)
#define CONV2_PKT_SIZE (CONV2_BUF_SIZE/4)

#define P2_OSIZE 4
#define P2_K 2

#define F1_N 500
#define F1_M 800
#define F1_P 10

#define F1_OUTLOOP 500
#define F1_LOOPEXE 250
#define F1_OUT_SIZE 500
#define F1_BUF_SIZE (F1_LOOPEXE+3)
#define F1_PKT_SIZE (F1_BUF_SIZE/4)

#define F2_N 10
#define F2_M 500
#define F2_P 10

#define F2_OUTLOOP 10
#define F2_LOOPEXE 5
#define F2_OUT_SIZE 10
#define F2_BUF_SIZE (F2_LOOPEXE+3)
#define F2_PKT_SIZE (F2_BUF_SIZE/4)

#define MAX_BUF_SIZE CONV1_BUF_SIZE
#define MAX_PKT_SIZE CONV1_PKT_SIZE

void mk2_buf_lenetall_2 (
	ap_fixed<169,69> sw1in[],
	ap_fixed<169,69> buf1[])
{
#pragma HLS INTERFACE axis register both port=sw1in
#pragma HLS INTERFACE axis register both port=buf1

int i;
ap_fixed<169,69> tmp[MAX_PKT_SIZE*(MBD-1)];
int pktnum;
static int state = 0;

if (state == 0) pktnum = CONV1_PKT_SIZE*(MBD-1);
else if (state == 1) pktnum = CONV2_PKT_SIZE*(MBD-1);
else if (state == 2) pktnum = F1_PKT_SIZE*(MBD-1);
else if (state == 3) pktnum = F2_PKT_SIZE*(MBD-1);


for(i=0; i<pktnum; i++) {
//	#pragma HLS PIPELINE
	tmp[i] = sw1in[i];
}

for(i=0; i<pktnum; i++) {
	//#pragma HLS PIPELINE
	buf1[i] = tmp[i];
}
if (state == 3) state = 0;
else state++;

return;
}

