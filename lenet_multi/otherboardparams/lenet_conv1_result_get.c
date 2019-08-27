#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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


#define MBD 4
#define C1_OCH 20
#define C1_OSIZE 24
#define C1_ICH 1
#define C1_ISIZE 28
#define C1_K 5

#define CONV1_LOOPEXE 6 // ceiling(CONV1_OUTLOOP/MBD)


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
	write_params("./params/conv1out_bd0.txt", tmp[0], C1_OCH*C1_ICH*C1_OSIZE*CONV1_LOOPEXE);	
	write_params("./params/conv1out_bd1.txt", tmp[1], C1_OCH*C1_ICH*C1_OSIZE*CONV1_LOOPEXE);	
	write_params("./params/conv1out_bd2.txt", tmp[2], C1_OCH*C1_ICH*C1_OSIZE*CONV1_LOOPEXE);	
	write_params("./params/conv1out_bd3.txt", tmp[3], C1_OCH*C1_ICH*C1_OSIZE*CONV1_LOOPEXE);	
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
	read_params("./txt/conv1_w.txt", conv1_w, CONV1_W_SIZE);
	print_params("CONV1_W : ", conv1_w, CONV1_W_SIZE);
	read_params("./txt/conv1_b.txt", conv1_b, CONV1_B_SIZE);
	print_params("CONV1_B : ", conv1_b, CONV1_B_SIZE);
	//Read CONV2 params
	read_params("./txt/conv2_w.txt", conv2_w, CONV2_W_SIZE);
	print_params("CONV2_W : ", conv2_w, CONV2_W_SIZE);
	read_params("./txt/conv2_b.txt", conv2_b, CONV2_B_SIZE);
	print_params("CONV2_B : ", conv2_b, CONV2_B_SIZE);
	//Read FC1 params
	read_params("./txt/fc1_w.txt", fc1_w, FC1_W_SIZE);
	print_params("FC1_W : ", fc1_w, FC1_W_SIZE);
	read_params("./txt/fc1_b.txt", fc1_b, FC1_B_SIZE);
	print_params("FC1_B : ", fc1_b, FC1_B_SIZE);
	//Read FC2 params
	read_params("./txt/fc2_w.txt", fc2_w, FC2_W_SIZE);
	print_params("FC2_W : ", fc2_w, FC2_W_SIZE);
	read_params("./txt/fc2_b.txt", fc2_b, FC2_B_SIZE);
	print_params("FC2_B : ", fc2_b, FC2_B_SIZE);

	printf("\n");

	//FEED-FORWARD
	printf("Feed forward ...\n\n");fflush(stdout);

	convolution(image, 28, 1, conv1_out, 24, 20, conv1_w, conv1_b, 5, 1);//CONV1
	//my_tanh(conv1_out, 24, 20);
	outputconv1(conv1_out);



	maxpooling(conv1_out, 24, 20, pool1_out, 12, 2, 2);//POOL1

	convolution(pool1_out, 12, 20, conv2_out, 8, 50, conv2_w, conv2_b, 5, 1);//CONV2
	//my_tanh(conv2_out, 8, 50);
  
	maxpooling(conv2_out, 8, 50, pool2_out, 4, 2, 2);//POOL2

    classifier(pool2_out, 800, fc1_out, 500, fc1_w, fc1_b);//FC1
	relu(fc1_out, 1, 500);

	classifier(fc1_out, 500, fc2_out, 10, fc2_w, fc2_b);//FC2
	softmax(fc2_out, 10);

	print_all_params(fc2_out, 10);//result
	
	//Compare between my outputs and caffe's outputs
	if (CHECK_PARAMS) {
		printf("Check params ...\n\n");fflush(stdout);
	}
	return;
}
