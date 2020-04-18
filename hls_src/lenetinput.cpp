#include "conv1_w.h"
#include "conv1_b.h"
#include "conv2_w.h"
#include "conv2_b.h"
#include "fc1_w.h"
#include "fc1_b.h"
#include "fc2_w.h"
#include "fc2_b.h"
#include "image000.h"

#define MBD 2 // number of boards
//conv1 parameter
#define C1_OCH 20
#define C1_OSIZE 24
#define C1_ICH 1
#define C1_ISIZE 28
#define C1_K 5
#define P1_OSIZE 12
#define P1_K 2

#define C2_OCH 50
#define C2_OSIZE 8
#define C2_ICH 20
#define C2_ISIZE 12
#define C2_K 5
#define C2_P 10

#define P2_OSIZE 4
#define P2_K 2

#define F1_N 500
#define F1_M 800
#define F1_P 10

#define F2_N 10
#define F2_M 500
#define F2_P 10
#define ALL_WB_SIZE (520+25050+400500+5010)

void lenetinput(
		float image[C1_ICH*C1_ISIZE*C1_ISIZE],
		float wb[ALL_WB_SIZE]
	) {

#pragma HLS INTERFACE axis register both port=image
#pragma HLS INTERFACE axis register both port=wb
	int i, j;
	float conv1_w[C1_OCH*C1_ICH*C1_K*C1_K] = CONV1_W;
	float conv1_b[C1_OCH] = CONV1_B;
	float conv2_w[C2_OCH*C2_ICH*C2_K*C2_K] = CONV2_W;
	float conv2_b[C2_OCH] = CONV2_B;
	float fc1_w[F1_N*F1_M] = FC1_W;
	float fc1_b[F1_N] = FC1_B;
	float fc2_w[F2_N*F2_M] = FC2_W;
	float fc2_b[F2_N] = FC2_B;
	float image000[C1_ICH*C1_ISIZE*C1_ISIZE] = IMAGE000;

	int offset = 0;
	for (j = 0; j < C1_OCH*C1_ICH*C1_K*C1_K; j++) wb[offset+j] = conv1_w[j];
	offset += C1_OCH*C1_ICH*C1_K*C1_K;
	for (j = 0; j < C1_OCH; j++) wb[offset+j] = conv1_b[j];
	offset += C1_OCH;
	for (j = 0; j < C2_OCH*C2_ICH*C2_K*C2_K; j++) wb[offset+j] = conv2_w[j];
	offset += C2_OCH*C2_ICH*C2_K*C2_K;
	for (j = 0; j < C2_OCH; j++) wb[offset+j] = conv2_b[j];
	offset += C2_OCH;

	for (j = 0; j < F1_N*F1_M; j++) wb[offset+j] = fc1_w[j];
	offset += F1_N*F1_M;
	for (j = 0; j < F1_N; j++) wb[offset+j] = fc1_b[j];
	offset += F1_N;
	for (j = 0; j < F2_N*F2_M; j++) wb[offset+j] = fc2_w[j];
	offset += F2_N*F2_M;
	for (j = 0; j < F2_N; j++) wb[offset+j] = fc2_b[j];
	offset += F2_N;

	for (j = 0; j < C1_ICH*C1_ISIZE*C1_ISIZE; j++) image[j] = image000[j]/255.0;
	//norm
	return;
}
