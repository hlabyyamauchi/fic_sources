#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ap_fixed.h> //only in C++
#include <ap_int.h> //only in C++

#include "/home/asap2/yyamauchi/lenettest/fic_sources/lenet_multi/headers/conv1_w.h"
#include "/home/asap2/yyamauchi/lenettest/fic_sources/lenet_multi/headers/conv1_b.h"
#include "/home/asap2/yyamauchi/lenettest/fic_sources/lenet_multi/headers/conv2_w.h"
#include "/home/asap2/yyamauchi/lenettest/fic_sources/lenet_multi/headers/conv2_b.h"
#include "/home/asap2/yyamauchi/lenettest/fic_sources/lenet_multi/headers/fc1_w.h"
#include "/home/asap2/yyamauchi/lenettest/fic_sources/lenet_multi/headers/fc1_b.h"
#include "/home/asap2/yyamauchi/lenettest/fic_sources/lenet_multi/headers/fc2_w.h"
#include "/home/asap2/yyamauchi/lenettest/fic_sources/lenet_multi/headers/fc2_b.h"
#include "/home/asap2/yyamauchi/lenettest/fic_sources/lenet_multi/headers/image000.h"

#define IMAGEFILE "/home/asap2/yyamauchi/lenettest/fic_sources/lenet_multi/weightandinput/image1st.txt"

#define CHECK_PARAMS 0

#define IMAGE_SIZE 1*28*28

#define CONV1_W_SIZE 20*1*5*5
#define CONV1_B_SIZE 20
//#define CONV1_OUT_SIZE 20*24*24

#define POOL1_OUT_SIZE 20*12*12

#define CONV2_W_SIZE 50*20*5*5
#define CONV2_B_SIZE 50
#define CONV2_OUT_SIZE 50*8*8

#define POOL2_OUT_SIZE 50*4*4

#define FC1_W_SIZE 500*800
#define FC1_B_SIZE 500
#define FC1_OUT_SIZE 500

#define FC2_W_SIZE 10*500
#define FC2_B_SIZE 10
#define FC2_OUT_SIZE 10

/* -- HLS define begin-- */
#define IN_DATASIZE 196
#define OUT_DATASIZE 3

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

#define P2_OSIZE 4
#define P2_K 2

#define F1_N 500
#define F1_M 800
#define F1_P 10

#define F2_N 10
#define F2_M 500

//#define RESULT_SIZE C1_OCH*C1_ICH*C1_OSIZE*C1_OSIZE
#define RESULT_SIZE 10

#define ALL_WB_SIZE (520+25050+400500+5010)
/////////////
//LeNet part parameter
#define MBD 2 // number of boards
//conv1 partition parameter
#define CONV1_OUTLOOP 24 //outer loop == C1_OSIZE
#define CONV1_LOOPEXE 12 // ceiling(CONV1_OUTLOOP/MBD)
#define CONV1_OUT_SIZE C1_OCH*C1_ICH*C1_OSIZE*C1_OSIZE
#define CONV1_BUF_SIZE (C1_OCH*C1_ICH*C1_OSIZE*CONV1_LOOPEXE+3)//+3 ha packet no tameno anaume
#define CONV1_PKT_SIZE (CONV1_BUF_SIZE/4) //

#define CONV2_OUTLOOP 8
#define CONV2_LOOPEXE 4
#define CONV2_BUF_SIZE (C2_OCH*C1_ICH*C2_OSIZE*CONV2_LOOPEXE+3)
#define CONV2_PKT_SIZE (CONV2_BUF_SIZE/4)
/////////////
/* -- HLS define end -- */
//////////
int max(int a, int b);
int min(int a, int b);

void print_params(char *name, float *array, int size);
void print_all_params(float *array, int size);
void read_params(char *path, float *array, int size);
void write_params(char *path, float *array, int size);
void check_params(float *array1, char *path, int size);
void read_binary(char *path, float *array, int size);
void write_binary(char *path, float *array, int size);
void check_binary(float *array1, char *path, int size);
void padding(float *input, int isize, int ichan, float *output, int pad);
void convolution(float *input, int isize, int ichan, float *output, int osize, int ochan, float *weight, float *bias, int ksize, int stride);
void maxpooling(float *input, int isize, int ichan, float *output, int osize,  int ksize, int stride);
void relu(float *input, int isize, int ichan);
void lrn(float *input, int isize, int ichan, float *output, int k, int n, float alpha, float beta);
void classifier(float *input, int isize, float *output, int osize, float *weight, float *bias);
void softmax(float *input, int isize);
void show_result(float *softmax, char *path, int size);
void norm_image(float *image, int size);
void show_image(float *normed_image, int xy_size);

void lenetall(
		float input[C1_ICH*C1_ISIZE*C1_ISIZE], //read all
		float output[RESULT_SIZE],//change here for debug
		float wb[ALL_WB_SIZE],
		ap_fixed<169,69> buf1[],
		ap_fixed<169,69> sw1out[],
		ap_fixed<169,69> sw2in[1],
		ap_fixed<169,69> sw2out[1],
		char id, // 8bit
		ap_uint<4> startt[1],
	    ap_uint<4> stopt[1]
	);
//////////
void my_tanh(float *input, int isize, int ichan) {
  int ocol, orow, och;

  printf("Tanh:\n");
  printf("  isize=%d, ichan=%d\n", isize, ichan);

  for (och= 0; och < ichan; och++) {
	for (orow = 0; orow < isize; orow++) {
	  for (ocol = 0; ocol < isize; ocol++) {
		  *(input+och*isize*isize+orow*isize+ocol) = tanhf(*(input+och*isize*isize+orow*isize+ocol));
		  //if (*(input+och*isize*isize+orow*isize+ocol) < 0.0) *(input+och*isize*isize+orow*isize+ocol) = 0.0;
	  }
	}
  }
  printf("\n");fflush(stdout);
}

int main() {
	int i, j, k, l;

	float *image;
	float *conv1_w, *conv1_b, *conv1_out;
	float *pool1_out;

	float *conv2_w, *conv2_b, *conv2_out;
	float *pool2_out;

	float *fc1_w, *fc1_b, *fc1_out;
	float *fc2_w, *fc2_b, *fc2_out;

	float *debug;

	float hls_idata[28*28];
	//float hls_wbdata[ALL_WB_SIZE+1];
	float hls_wbdata[ALL_WB_SIZE];
	float hls_odata[RESULT_SIZE], hls_result[RESULT_SIZE];
	printf("/// LeNet ///\n\n");fflush(stdout);
	printf("Memory allocation ...\n");fflush(stdout);
	if ((image = (float *)malloc(sizeof(float)*IMAGE_SIZE)) == NULL ||
		(conv1_w = (float *)malloc(sizeof(float)*CONV1_W_SIZE)) == NULL ||
		(conv1_b = (float *)malloc(sizeof(float)*CONV1_B_SIZE)) == NULL ||
		(conv1_out = (float *)malloc(sizeof(float)*CONV1_OUT_SIZE)) == NULL ||
		(pool1_out = (float *)malloc(sizeof(float)*POOL1_OUT_SIZE)) == NULL ||
		(conv2_w = (float *)malloc(sizeof(float)*CONV2_W_SIZE)) == NULL ||
		(conv2_b = (float *)malloc(sizeof(float)*CONV2_B_SIZE)) == NULL ||
		(conv2_out = (float *)malloc(sizeof(float)*CONV2_OUT_SIZE)) == NULL ||
		(pool2_out = (float *)malloc(sizeof(float)*POOL2_OUT_SIZE)) == NULL ||
		(fc1_w = (float *)malloc(sizeof(float)*FC1_W_SIZE)) == NULL ||
		(fc1_b = (float *)malloc(sizeof(float)*FC1_B_SIZE)) == NULL ||
		(fc1_out = (float *)malloc(sizeof(float)*FC1_OUT_SIZE)) == NULL ||
		(fc2_w = (float *)malloc(sizeof(float)*FC2_W_SIZE)) == NULL ||
		(fc2_b = (float *)malloc(sizeof(float)*FC2_B_SIZE)) == NULL ||
		(fc2_out = (float *)malloc(sizeof(float)*FC2_OUT_SIZE)) == NULL ||
		0) {
		printf("MemError\n");
		exit(1);
	}
	printf("\n");
////////////LeNet part/////////////
	float *bufs[MBD-1];
	float *bufsconv2[MBD-1];
	ap_fixed<169,69> buf1[CONV1_PKT_SIZE*(MBD-1)+CONV2_PKT_SIZE*(MBD-1)];
	ap_fixed<169,69> sw1out[CONV1_PKT_SIZE];
	for (i = 0; i < MBD-1; i++) {
		if ((bufs[i] = (float *)malloc(sizeof(float)*CONV1_BUF_SIZE)) == NULL ||
				(bufsconv2[i] = (float *)malloc(sizeof(float)*CONV2_BUF_SIZE))== NULL ||
				0) {
			printf("MemError\n");
			exit(1);
		}
	}
	for (i = 0; i < MBD-1; i++) {
		for (j = 0; j < CONV1_BUF_SIZE; j++) {
			bufs[i][j] = 0;
		}
		for (j = 0; j < CONV2_BUF_SIZE; j++) {
					bufsconv2[i][j] = 0;
				}
	}
	ap_fixed<169,69> sw2in[1];
	ap_fixed<169,69> sw2out[1];
	char id;
	ap_uint<4> startt[1];
    ap_uint<4> stopt[1];

	printf("Read other boards params ...\n\n");fflush(stdout);
	read_params("/home/asap2/yyamauchi/lenettest/fic_sources/lenet_multi/otherboardparams/params/lenet2conv1out_bd1.txt", bufs[0], C1_OCH*C1_OSIZE*CONV1_LOOPEXE);
	//read_params("./otherboardparams/params/conv1out_bd2.txt", bufs[1], C1_OCH*C1_ICH*C1_OSIZE*CONV1_LOOPEXE);
	//read_params("./otherboardparams/params/conv1out_bd3.txt", bufs[2], C1_OCH*C1_ICH*C1_OSIZE*CONV1_LOOPEXE);
	read_params("/home/asap2/yyamauchi/lenettest/fic_sources/lenet_multi/otherboardparams/params/lenet2conv2out_bd1.txt", bufsconv2[0], C2_OCH*C2_OSIZE*CONV2_LOOPEXE);
	printf("ok!\n");
	ap_fixed<169,69> packet = 0;
	ap_uint<16> head;
	int board;
	for (board = 0; board < MBD-1; board++) {
		head = (ap_uint<16>)(board+1);
		for (i = 0, j = 0; j < CONV1_PKT_SIZE; i+=4, j++) {
			#pragma HLS PIPELINE II=1
			packet(168,153) = head(15,0);
			packet(127,96) = ((ap_fixed<32,16>)bufs[board][i])(31,0);
			packet(95,64) =((ap_fixed<32,16>)bufs[board][i+1])(31,0);
			packet(63,32) = ((ap_fixed<32,16>)bufs[board][i+2])(31,0);
			packet(31,0) =((ap_fixed<32,16>)bufs[board][i+3])(31,0);
			buf1[board*CONV1_PKT_SIZE +j] = packet;
		}
	}
	for (board = 0; board < MBD-1; board++) {
		head = (ap_uint<16>)(board+1);
		for (i = 0, j = 0; j < CONV2_PKT_SIZE; i+=4, j++) {
				#pragma HLS PIPELINE II=1
				packet(168,153) = head(15,0);
				packet(127,96) = ((ap_fixed<32,16>)bufsconv2[board][i])(31,0);
				packet(95,64) =((ap_fixed<32,16>)bufsconv2[board][i+1])(31,0);
				packet(63,32) = ((ap_fixed<32,16>)bufsconv2[board][i+2])(31,0);
				packet(31,0) =((ap_fixed<32,16>)bufsconv2[board][i+3])(31,0);
				buf1[(MBD-1)*CONV1_PKT_SIZE + board*CONV2_PKT_SIZE +j] = packet;
			}
	}
	id = (char)0;
	startt[0] = (ap_uint<4>)1;
//////////////////////////////////
	printf("Read params ...\n\n");fflush(stdout);
	read_params(IMAGEFILE, image, IMAGE_SIZE);
	norm_image(image, IMAGE_SIZE);
//show iamge
	for (i = 0; i < 28; i++) {
		for (j = 0; j < 28; j++) {
			if (*(image+i*28+j) > 0.5){
				printf ("* ");
			} else {
				printf("  ");
			}
		}
		printf ("\n");
	}

	print_params("IMAGE : ", image, IMAGE_SIZE);
	//Read CONV1 params
	read_params("/home/asap2/yyamauchi/lenettest/fic_sources/lenet_multi/weightandinput/conv1_w.txt", conv1_w, CONV1_W_SIZE);
	print_params("CONV1_W : ", conv1_w, CONV1_W_SIZE);
	read_params("/home/asap2/yyamauchi/lenettest/fic_sources/lenet_multi/weightandinput/conv1_b.txt", conv1_b, CONV1_B_SIZE);
	print_params("CONV1_B : ", conv1_b, CONV1_B_SIZE);
	//Read CONV2 params
	read_params("/home/asap2/yyamauchi/lenettest/fic_sources/lenet_multi/weightandinput/conv2_w.txt", conv2_w, CONV2_W_SIZE);
	print_params("CONV2_W : ", conv2_w, CONV2_W_SIZE);
	read_params("/home/asap2/yyamauchi/lenettest/fic_sources/lenet_multi/weightandinput/conv2_b.txt", conv2_b, CONV2_B_SIZE);
	print_params("CONV2_B : ", conv2_b, CONV2_B_SIZE);
	//Read FC1 params
	read_params("/home/asap2/yyamauchi/lenettest/fic_sources/lenet_multi/weightandinput/fc1_w.txt", fc1_w, FC1_W_SIZE);
	print_params("FC1_W : ", fc1_w, FC1_W_SIZE);
	read_params("/home/asap2/yyamauchi/lenettest/fic_sources/lenet_multi/weightandinput/fc1_b.txt", fc1_b, FC1_B_SIZE);
	print_params("FC1_B : ", fc1_b, FC1_B_SIZE);
	//Read FC2 params
	read_params("/home/asap2/yyamauchi/lenettest/fic_sources/lenet_multi/weightandinput/fc2_w.txt", fc2_w, FC2_W_SIZE);
	print_params("FC2_W : ", fc2_w, FC2_W_SIZE);
	read_params("/home/asap2/yyamauchi/lenettest/fic_sources/lenet_multi/weightandinput/fc2_b.txt", fc2_b, FC2_B_SIZE);
	print_params("FC2_B : ", fc2_b, FC2_B_SIZE);

	printf("\n");

	//FEED-FORWARD
	printf("Feed forward ...\n\n");fflush(stdout);
	convolution(image, 28, 1, conv1_out, 24, 20, conv1_w, conv1_b, 5, 1);//CONV1
	//my_tanh(conv1_out, 24, 20);
	maxpooling(conv1_out, 24, 20, pool1_out, 12, 2, 2);//POOL1
	convolution(pool1_out, 12, 20, conv2_out, 8, 50, conv2_w, conv2_b, 5, 1);//CONV2
	//my_tanh(conv2_out, 8, 50);
	maxpooling(conv2_out, 8, 50, pool2_out, 4, 2, 2);//POOL2
    classifier(pool2_out, 800, fc1_out, 500, fc1_w, fc1_b);//FC1
	relu(fc1_out, 1, 500);
	classifier(fc1_out, 500, fc2_out, 10, fc2_w, fc2_b);//FC2
	softmax(fc2_out, 10);
	printf("ok1\n");
	print_all_params(fc2_out, 10);//result
	printf("ok1\n");

	float inputimage[C1_ICH*C1_ISIZE*C1_ISIZE];
	float wb[ALL_WB_SIZE];
	float conv1_w2[C1_OCH*C1_ICH*C1_K*C1_K] = CONV1_W;
	float conv1_b2[C1_OCH] = CONV1_B;
	float conv2_w2[C2_OCH*C2_ICH*C2_K*C2_K] = CONV2_W;
	float conv2_b2[C2_OCH] = CONV2_B;
	float fc1_w2[F1_N*F1_M] = FC1_W;
	float fc1_b2[F1_N] = FC1_B;
	float fc2_w2[F2_N*F2_M] = FC2_W;
	float fc2_b2[F2_N] = FC2_B;
	float image000[C1_ICH*C1_ISIZE*C1_ISIZE] = IMAGE000;

	int offset = 0;
	for (j = 0; j < C1_OCH*C1_ICH*C1_K*C1_K; j++) wb[offset+j] = conv1_w2[j];
	offset += C1_OCH*C1_ICH*C1_K*C1_K;
	for (j = 0; j < C1_OCH; j++) wb[offset+j] = conv1_b2[j];
	offset += C1_OCH;
	for (j = 0; j < C2_OCH*C2_ICH*C2_K*C2_K; j++) wb[offset+j] = conv2_w2[j];
	offset += C2_OCH*C2_ICH*C2_K*C2_K;
	for (j = 0; j < C2_OCH; j++) wb[offset+j] = conv2_b2[j];
	offset += C2_OCH;

	for (j = 0; j < F1_N*F1_M; j++) wb[offset+j] = fc1_w2[j];
	offset += F1_N*F1_M;
	for (j = 0; j < F1_N; j++) wb[offset+j] = fc1_b2[j];
	offset += F1_N;
	for (j = 0; j < F2_N*F2_M; j++) wb[offset+j] = fc2_w2[j];
	offset += F2_N*F2_M;
	for (j = 0; j < F2_N; j++) wb[offset+j] = fc2_b2[j];
	offset += F2_N;

	for (j = 0; j < C1_ICH*C1_ISIZE*C1_ISIZE; j++) inputimage[j] = image000[j] / 255.0;

	lenetall(inputimage, hls_odata, wb, buf1, sw1out, sw2in, sw2out, id, startt, stopt);
	k = 0;
	for (i = 0; i < 10; i++) {
		if (abs(*(fc2_out+i) - *(hls_result+i)) > 0.01) {
			printf("ERROR :%d (%f, %f)\n", i, *(fc2_out+i), *(hls_result+i));
			k++;
		}
	}
	printf("total error=%d\n GO HLS FEEDFORWARD\n", k);
	print_all_params(hls_odata, RESULT_SIZE);//result

	/* HLS debug space END*/
	//Compare between my outputs and caffe's outputs
	if (CHECK_PARAMS) {
		printf("Check params ...\n\n");fflush(stdout);
	}
	return 0;
}
