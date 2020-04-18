#define IMAGESIZE (28*28)
#define ALL_WB_SIZE (520+25050+400500+5010)
#define RESULTSIZE 10

void lenetall_io(
		unsigned int rasin[],
		unsigned int rasout[RESULTSIZE],
		unsigned int image[IMAGESIZE],
		unsigned int wb[ALL_WB_SIZE],
		unsigned int result[RESULTSIZE]
		){
#pragma HLS INTERFACE axis register both port=rasin
#pragma HLS INTERFACE axis register both port=rasout
#pragma HLS INTERFACE axis register both port=image
#pragma HLS INTERFACE axis register both port=wb
#pragma HLS INTERFACE axis register both port=result
	int i;
	static unsigned int count = 0;

	if (count == 0) {
		for (i = 0; i < ALL_WB_SIZE; i++) {
			wb[i] = rasin[i];
		}
	} else if (count > 0) {
		for (i = 0; i < IMAGESIZE; i++) {
			image[i] = rasin[i];
		}
		for (i = 0; i < RESULTSIZE; i++) {
			rasout[i] = result[i];
		}
	}
	count++;
	return;
}
