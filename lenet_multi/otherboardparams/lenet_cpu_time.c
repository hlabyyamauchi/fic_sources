#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>

#define IMAGEFILE "./txt/image1st.txt"
#define CHECK_PARAMS 0

#define IMAGE_SIZE 1*28*28

#define CONV1_W_SIZE 20*1*5*5
#define CONV1_B_SIZE 20
#define CONV1_OUT_SIZE 20*24*24

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


#define MBD 2
#define C1_OCH 20
#define C1_OSIZE 24
#define C1_ICH 1
#define C1_ISIZE 28
#define C1_K 5
#define CONV1_LOOPEXE  12// ceiling(CONV1_OUTLOOP/MBD)

#define C2_OCH 50
#define C2_OSIZE 8
#define C2_ICH 20
#define C2_ISIZE 12
#define C2_K 5
#define C2_P 10
#define CONV2_LOOPEXE 4

#define F1_LOOPEXE 250
#define F2_LOOPEXE 5

void outputconv1(float *conv1){
	int i, j, k, l, bid;
  	float tmp[MBD][C1_OCH*C1_ICH*C1_OSIZE*CONV1_LOOPEXE];
	for (bid = 0; bid < MBD; bid++) {
		l = 0;
		for (j = bid*CONV1_LOOPEXE; j < (bid+1)*CONV1_LOOPEXE; j++) {
			for (i = 0; i < C1_OSIZE; i++) {
				for (k = 0; k < C1_OCH; k++) {
				  	tmp[bid][l] = *(conv1+k*C1_OSIZE*C1_OSIZE+j*C1_OSIZE+i);
				  	l++;
				}
			}
		}
	}
	write_params("./params/lenet2conv1out_bd0.txt", tmp[0], C1_OCH*C1_OSIZE*CONV1_LOOPEXE);	
	write_params("./params/lenet2conv1out_bd1.txt", tmp[1], C1_OCH*C1_OSIZE*CONV1_LOOPEXE);	
	return;
}
void outputconv2(float *conv2){
	int i, j, k, l, bid;
  	float tmp[MBD][C2_OCH*C2_ICH*C2_OSIZE*CONV2_LOOPEXE];
	for (bid = 0; bid < MBD; bid++) {
		l = 0;
		for (j = bid*CONV2_LOOPEXE; j < (bid+1)*CONV2_LOOPEXE; j++) {
			for (i = 0; i < C2_OSIZE; i++) {
				for (k = 0; k < C2_OCH; k++) {
				  	tmp[bid][l] = *(conv2+k*C2_OSIZE*C2_OSIZE+j*C2_OSIZE+i);
				  	l++;
				}
			}
		}
	}
	write_params("./params/lenet2conv2out_bd0.txt", tmp[0], C2_OCH*C2_OSIZE*CONV2_LOOPEXE);	
	write_params("./params/lenet2conv2out_bd1.txt", tmp[1], C2_OCH*C2_OSIZE*CONV2_LOOPEXE);	
	return;
}
void outputfc1(float *fc1) {
	float tmp[MBD][F1_LOOPEXE];
	int i, bid, num;
	for (bid = 0; bid < MBD; bid++) {
		num = 0; 
		for (i = bid*F1_LOOPEXE; i < (bid+1)*F1_LOOPEXE; i++) {
			tmp[bid][num] = *(fc1+i);
			num++;
		}
	}
	write_params("./params/lenet2fc1out_bd0.txt", tmp[0], F1_LOOPEXE);	
	write_params("./params/lenet2fc1out_bd1.txt", tmp[1], F1_LOOPEXE);	
	return;
}
void outputfc2(float *fc2) {
	float tmp[MBD][F2_LOOPEXE];
	int i, bid, num;
	for (bid = 0; bid < MBD; bid++) {
		num = 0; 
		for (i = bid*F2_LOOPEXE; i < (bid+1)*F2_LOOPEXE; i++) {
			tmp[bid][num] = *(fc2+i);
			num++;
		}
	}
	write_params("./params/lenet2fc2out_bd0.txt", tmp[0], F2_LOOPEXE);	
	write_params("./params/lenet2fc2out_bd1.txt", tmp[1], F2_LOOPEXE);	
	return;
}


void main() {
	int i, j, k, l;

	float *image;
	float *conv1_w, *conv1_b, *conv1_out;
	float *pool1_out;
  
	float *conv2_w, *conv2_b, *conv2_out;
	float *pool2_out;

	float *fc1_w, *fc1_b, *fc1_out;
	float *fc2_w, *fc2_b, *fc2_out;

	float *debug;

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

	printf("Read params ...\n\n");fflush(stdout);
	//Read image data

/*
//debug_space begin
	for (i = 0; i < IMAGE_SIZE; i++) {
		*(image+i) = 0.0;
		if (12 < (i%28) && (i%28) < 16) {
			*(image+i) = 1.0;
		}
	}
//end
*/
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
//	struct timeval start, end;
//	gettimeofday(&start, NULL);

	
	//print_params("IMAGE : ", image, IMAGE_SIZE);
	//Read CONV1 params
	read_params("./txt/conv1_w.txt", conv1_w, CONV1_W_SIZE);
	//print_params("CONV1_W : ", conv1_w, CONV1_W_SIZE);
	read_params("./txt/conv1_b.txt", conv1_b, CONV1_B_SIZE);
	//print_params("CONV1_B : ", conv1_b, CONV1_B_SIZE);
	//Read CONV2 params
	read_params("./txt/conv2_w.txt", conv2_w, CONV2_W_SIZE);
	//print_params("CONV2_W : ", conv2_w, CONV2_W_SIZE);
	read_params("./txt/conv2_b.txt", conv2_b, CONV2_B_SIZE);
	//print_params("CONV2_B : ", conv2_b, CONV2_B_SIZE);
	//Read FC1 params
	read_params("./txt/fc1_w.txt", fc1_w, FC1_W_SIZE);
	//print_params("FC1_W : ", fc1_w, FC1_W_SIZE);
	read_params("./txt/fc1_b.txt", fc1_b, FC1_B_SIZE);
	//print_params("FC1_B : ", fc1_b, FC1_B_SIZE);
	//Read FC2 params
	read_params("./txt/fc2_w.txt", fc2_w, FC2_W_SIZE);
	//print_params("FC2_W : ", fc2_w, FC2_W_SIZE);
	read_params("./txt/fc2_b.txt", fc2_b, FC2_B_SIZE);
	//print_params("FC2_B : ", fc2_b, FC2_B_SIZE);
	//FEED-FORWARD
	struct timeval start, end;
	gettimeofday(&start, NULL);
	read_params(IMAGEFILE, image, IMAGE_SIZE);
	norm_image(image, IMAGE_SIZE);
	convolution(image, 28, 1, conv1_out, 24, 20, conv1_w, conv1_b, 5, 1);//CONV1
	//my_tanh(conv1_out, 24, 20);
	outputconv1(conv1_out);
	maxpooling(conv1_out, 24, 20, pool1_out, 12, 2, 2);//POOL1
	convolution(pool1_out, 12, 20, conv2_out, 8, 50, conv2_w, conv2_b, 5, 1);//CONV2
	//my_tanh(conv2_out, 8, 50);
	outputconv2(conv2_out);
	maxpooling(conv2_out, 8, 50, pool2_out, 4, 2, 2);//POOL2
    	classifier(pool2_out, 800, fc1_out, 500, fc1_w, fc1_b);//FC1
	relu(fc1_out, 1, 500);
	classifier(fc1_out, 500, fc2_out, 10, fc2_w, fc2_b);//FC2
	softmax(fc2_out, 10);
	gettimeofday(&end, NULL);
	print_all_params(fc2_out, 10);//result
	printf("time:%lfsec\n", (end.tv_sec-start.tv_sec)+(double)(end.tv_usec-start.tv_usec)/1000000);	
	//Compare between my outputs and caffe's outputs
	if (CHECK_PARAMS) {
		printf("Check params ...\n\n");fflush(stdout);
	}
	return;
}
