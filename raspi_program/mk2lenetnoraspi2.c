#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

#include "mk2_fic_rpi.h"

#define BUFSIZE 2000000

#define IMAGEFILE "/home/asap/yyamauchi/vivado_workspace/cgh960_8/rasbpi/lenet_txt/image1000/"
#define CATEFILE "/home/asap/yyamauchi/vivado_workspace/cgh960_8/rasbpi//lenet_txt/category.txt"

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

#define RESULT_SIZE 10

#define ALL_WB_SIZE (CONV1_W_SIZE+CONV1_B_SIZE+CONV2_W_SIZE+CONV2_B_SIZE+FC1_W_SIZE+FC1_B_SIZE+FC2_W_SIZE+FC2_B_SIZE)

void print_params(char *name, float *array, int size);
void read_params(char *path, float *array, int size);
void norm_image(float *image, int size);
void show_image(float *normed_image, int xy_size);
void show_result(float *softmax, char *path, int size);

union float_and_uint {
	float fval;
	unsigned int ival;
};

int main(int argc, char *argv[]) {
	int i, j, k, l, cnt = 0;
	
	int map_status;
	int data;
	int c;
	char s[32];
	char imagefile[64], cntstr[10];
	
	float *image;
	float *conv1_w, *conv1_b, *conv1_out;
	float *pool1_out;
  
	float *conv2_w, *conv2_b, *conv2_out;
	float *pool2_out;

	float *fc1_w, *fc1_b, *fc1_out;
	float *fc2_w, *fc2_b, *fc2_out;

	float *result;

	float *debug;

	float *wb;

	unsigned int tmp, wcount, rcount;
	union float_and_uint wb_tmp;
	if ((result = (float *)malloc(sizeof(float)*RESULT_SIZE)) == NULL || 0) {
		printf("MemError\n");
		exit(1);
	}
	
	printf("/// LeNet ///\n\n");fflush(stdout);
	printf("Map GPIO ...\n\n");fflush(stdout);	
	rpi_gpio gpio = {GPIO_BASE};
	map_status = MapGPIO(&gpio);
	if(map_status) {
		printf("Failed to map GPIO.\n");
		return map_status;
	}
	init_test(&gpio );
	data = 0x000; write_test(&gpio,data);  // stb=1 Write

	
	printf("Reset FPGA modules ...\n\n");fflush(stdout);	
	comwrite(gpio, 4);
	sleep(1);
	
	printf("Start FPGA modules ...\n\n");fflush(stdout);
	comwrite(gpio, 1);
 // 	sleep(1);



	printf("\n");
///////////wb_write end///////////////////
///////////read 1 for debug////////////////
///////////read 2 end for debug////////////////
///////////read result start/////////////////////////
		printf("Read data from FPGA ...\n\n");fflush(stdout);
		comwrite(gpio, 3);
		//usleep(1000);
//		printf("address=0x1000\n");fflush(stdout);
		addwrite(gpio, 0x1000);
		rcount = RESULT_SIZE; // RCOUNT
		init_read(&gpio );
		for(i=0; i<rcount; i++) {
//			printf("Read Data: ");
			wb_tmp.ival &= 0x00000000;
			for (l = 0; l < 2; l++) {
				tmp = read_cont(gpio);
				wb_tmp.ival |= ((tmp & 0x0000000f) << (16-l*16));
//				printf("%x ", tmp);
			}
//			printf(" = %f\n", wb_tmp.fval);fflush(stdout);
			*(result+i) = wb_tmp.fval;
		}
		read_fin(gpio);

		show_result(result, CATEFILE, 10);

		cnt++;
		if (cnt == 1000) cnt = 0;
		
//printf("\n\n");fflush(stdout);
	init_test(&gpio);

	UnmapGPIO(&gpio);
	
	printf("\n");
	
	printf("Finished\n");fflush(stdout);
	
	return 0;
}
