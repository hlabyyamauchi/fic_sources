#include <ap_fixed.h>
#include <ap_int.h> //only in C++

#include<math.h>
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
#define CONV1_OUT_SIZE C1_OCH*C1_OSIZE*C1_OSIZE
#define CONV1_BUF_SIZE (C1_OCH*C1_OSIZE*CONV1_LOOPEXE+3)//means CONV1_OUT_SIZE/CONV1_OUTLOOP*CONV1_LOOPEXE   +3 ceiling for pkt
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

#define RESULTSIZE 10

#define ALL_WB_SIZE (520+25050+400500+5010)

void load_input(
		float input[C1_ICH*C1_ISIZE*C1_ISIZE],
		float output[C1_ICH][C1_ISIZE][C1_ISIZE]
		){
#pragma HLS INLINE off
	int i, j, k;
	float tmp[C1_ICH*C1_ISIZE*C1_ISIZE];

	for (i = 0; i < C1_ICH*C1_ISIZE*C1_ISIZE; i++) {
		tmp[i] = input[i];
	}

	for(i = 0; i < C1_ICH; i++) {
		for(j = 0; j < C1_ISIZE; j++) {
			for(k = 0; k < C1_ISIZE; k++) {
				output[i][j][k] = tmp[i*C1_ISIZE*C1_ISIZE + j*C1_ISIZE + k];
			}
		}
	}
	return;
}

void load_wb(
		float input[ALL_WB_SIZE],
		float conv1_w[C1_OCH][C1_ICH][C1_K][C1_K],
		float conv1_b[C1_OCH],
		float conv2_w[C2_OCH][C2_ICH][C2_K][C2_K],
		float conv2_b[C2_OCH],
		float fc1_w[F1_N][F1_M],
		float fc1_b[F1_N],
		float fc2_w[F2_N][F2_M],
		float fc2_b[F2_N]
		){
#pragma HLS INLINE off
	int i, j, k, l;
	unsigned long datasize, offset;
	//CONV1_WB
	offset = 0;
	for(i = 0; i < C1_OCH; i++) {
		for(j = 0; j < C1_ICH; j++) {
			for(k = 0; k < C1_K; k++) {
				for (l = 0; l < C1_K; l++) {
					conv1_w[i][j][k][l] = input[offset + i*C1_ICH*C1_K*C1_K + j*C1_K*C1_K + k*C1_K + l];
				}
			}
		}
	}
	for(i = 0; i < C1_OCH; i++) {
		conv1_b[i] = input[offset + C1_OCH*C1_ICH*C1_K*C1_K + i];
	}

	//CONV2_WB
	offset = 520;
	for(i = 0; i < C2_OCH; i++) {
		for(j = 0; j < C2_ICH; j++) {
			for(k = 0; k < C2_K; k++) {
				for (l = 0; l < C2_K; l++) {
					conv2_w[i][j][k][l] = input[offset + i*C2_ICH*C2_K*C2_K + j*C2_K*C2_K + k*C2_K + l];
				}
			}
		}
	}
	for(i = 0; i < C2_OCH; i++) {
		conv2_b[i] = input[offset + C2_OCH*C2_ICH*C2_K*C2_K + i];
	}

	//FC1_WB
	offset = 520 + 25050;
	for(i = 0; i < F1_N; i++)
		for (j = 0; j < F1_M; j++)
			fc1_w[i][j] = input[offset + i*F1_M + j];
	for(i = 0; i < F1_N; i++)
		fc1_b[i] = input[offset + F1_N*F1_M + i];

	//FC1_WB
	offset = 520 + 25050 + 400500;
	for(i = 0; i < F2_N; i++)
		for (j = 0; j < F2_M; j++)
			fc2_w[i][j] = input[offset + i*F2_M + j];
	for(i = 0; i < F2_N; i++)
		fc2_b[i] = input[offset + F2_N*F2_M + i];

	return;
}

void conv1_part(
		float input[C1_ICH][C1_ISIZE][C1_ISIZE],
		float weight[C1_OCH][C1_ICH][C1_K][C1_K],
		float bias[C1_OCH],
		float buffer[MAX_BUF_SIZE],
		ap_uint<16> idd
) {
#pragma HLS INLINE off
	int ox, oy, kx, ky, n, m;
	int i = 0;
	static int stride = 1;
		for (ox = 0 ; ox < C1_OSIZE; ox++) {
			if ((ox < idd * CONV1_LOOPEXE) || ((idd+1) * CONV1_LOOPEXE <= ox)) continue;
			for (oy = 0; oy < C1_OSIZE; oy++) {
				for (n = 0; n < C1_OCH; n++) {
					buffer[i] = bias[n];
					for (m = 0; m < C1_ICH; m++) {
						for (kx = 0; kx < C1_K; kx++) {
							for (ky = 0; ky < C1_K; ky++) {
								buffer[i] += weight[n][m][kx][ky] * input[m][stride*ox+kx][stride*oy+ky];
							}
						}
					}
					i++;
				}
			}
		}
	return;
}

void data_trans(
	int sendnum,
	float senddata[],
	ap_uint<16> idd,
	ap_fixed<169,69> tmpout[],
	ap_fixed<169,69> output[]
) {
	ap_fixed<169,69> packet = 0;
	ap_uint<16> head;
	int i, j, k;
	i = 0;
	head = idd;
	for (i = 0, j = 0; j < sendnum; i+=4, j++) {
			#pragma HLS PIPELINE II=1
			packet(168,153) = head(15,0);
			packet(127,96) = ((ap_fixed<32,16>)senddata[i])(31,0); 
			packet(95,64) =((ap_fixed<32,16>)senddata[i+1])(31,0); 
			packet(63,32) = ((ap_fixed<32,16>)senddata[i+2])(31,0); 
			packet(31,0) =((ap_fixed<32,16>)senddata[i+3])(31,0); 
			tmpout[j] = packet;
	}
	for (k = 0; k < sendnum; k++) output[k] = tmpout[k]; //send data
	return;
}

void data_recv(
	int sendnum,
	int inputnum,
	float v[MBD][MAX_BUF_SIZE],//recvdata
	ap_uint<16> idd,
	ap_fixed<169,69> tmpin[],
	ap_fixed<169,69> input[]
) {
	int i;
	ap_uint<16> pid;
	ap_fixed<32,16> tmp0, tmp1, tmp2, tmp3;
	int ii[MBD]; //receive counter
	for(i=0; i<sendnum*(MBD-1); i++) tmpin[i] = input[inputnum + i]; //receive data
	for(i=0; i<MBD; i++) ii[i]=0;
	for(i=0; i<sendnum*(MBD-1); i++) {
#pragma HLS PIPELINE II=1
		pid(15,0) = tmpin[i](168,153);
	    tmp0(31,0) = tmpin[i](127,96);
	    v[pid][ii[pid]] = (float)tmp0;
	    tmp1(31,0) = tmpin[i](95,64);
	    v[pid][ii[pid]+1] = (float)tmp1;
	    tmp2(31,0) = tmpin[i](63,32);
	    v[pid][ii[pid]+2] = (float)tmp2;
	    tmp3(31,0) = tmpin[i](31,0);
	    v[pid][ii[pid]+3] = (float)tmp3;
	    ii[pid] = ii[pid] +4;
	}
	return;
}


void conv1_r(
	int sendnum,
	float v[MBD][MAX_BUF_SIZE],//recvdata
	ap_fixed<169,69> input[],
	ap_fixed<169,69> tmpin[],
	ap_uint<16> idd,
	float conv1_out[C1_OCH][C1_OSIZE][C1_OSIZE],
	float buffer[MAX_BUF_SIZE]
) {
	int board;
	int ox, oy, n, num;
	int inputnum = 0;
	data_recv(sendnum, inputnum, v, idd, tmpin, input); //data receive
	//arrange results into conv1_out
	for (board = 0; board < MBD; board++) {
		num = 0;
		for (ox = 0 ; ox < C1_OSIZE; ox++) {
			if ((ox < board * CONV1_LOOPEXE) || ((board+1) * CONV1_LOOPEXE <= ox)) continue;
			for (oy = 0; oy < C1_OSIZE; oy++) {
				for (n = 0; n < C1_OCH; n++) {
					if (board == idd) conv1_out[n][ox][oy] = buffer[num];
					else conv1_out[n][ox][oy] = v[board][num];
					num++;
				}
			}
		}
	}
	return;
}

void conv1_all(
		float input[C1_ICH][C1_ISIZE][C1_ISIZE],
		float weight[C1_OCH][C1_ICH][C1_K][C1_K],
		float bias[C1_OCH],
		float conv1_out[C1_OCH][C1_OSIZE][C1_OSIZE],
		ap_uint<16> idd,
		float buffer[MAX_BUF_SIZE],
	 	float v[MBD][MAX_BUF_SIZE],
	 	ap_fixed<169,69> tmpout[MAX_PKT_SIZE],
	 	ap_fixed<169,69> tmpin[MAX_PKT_SIZE*(MBD-1)],
		ap_fixed<169,69> tdata[],
		ap_fixed<169,69> rdata[]
) {
	 int j, k;
	 int sendnum = CONV1_PKT_SIZE;
	 for (j = 0; j < MAX_BUF_SIZE; j++) buffer[j] = 0;
	 for (j = 0; j < MBD; j++)
		 for(k = 0; k < MAX_BUF_SIZE; k++) v[j][k] = 0;
	conv1_part(input, weight, bias, buffer, idd);
	data_trans(sendnum, buffer, idd, tmpout, tdata);
	conv1_r(sendnum, v, rdata, tmpin, idd, conv1_out, buffer);
	int i;
	return;
}
void conv1(
		float input[C1_ICH][C1_ISIZE][C1_ISIZE],
		float weight[C1_OCH][C1_ICH][C1_K][C1_K],
		float bias[C1_OCH],
		float output[C1_OCH][C1_OSIZE][C1_OSIZE]
) {
#pragma HLS INLINE off
	int ox, oy, kx, ky, n, m;
	static int stride = 1;
		for (ox = 0; ox < C1_OSIZE; ox++) {
			for (oy = 0; oy < C1_OSIZE; oy++) {
				for (n = 0; n < C1_OCH; n++) {
					output[n][ox][oy] = bias[n];
					for (m = 0; m < C1_ICH; m++) {
						for (kx = 0; kx < C1_K; kx++) {
							for (ky = 0; ky < C1_K; ky++) {
								output[n][ox][oy] +=
										weight[n][m][kx][ky] *
										input[m][stride*ox+kx][stride*oy+ky];

							}
						}
					}
				}
			}
		}
	return;
}

void store_output_debug(float conv1_out[C1_OCH][C1_OSIZE][C1_OSIZE], float output[]){
#pragma HLS INLINE off
	int i = 0;
	int ox, oy, n, m;
	for (ox = 0; ox < C1_OSIZE; ox++) {
			for (oy = 0; oy < C1_OSIZE; oy++) {
				for (n = 0; n < C1_OCH; n++) {
						output[i] = conv1_out[n][ox][oy];
						i++;
				}
			}
		}
	return;
}
void pool1(
		float input[C1_OCH][C1_OSIZE][C1_OSIZE],
		float output[C1_OCH][P1_OSIZE][P1_OSIZE]
		){
#pragma HLS INLINE off
	int ox, oy, kx, ky, ix, iy, n, m;
	float tmp, max;

	int stride = 2;
	  for (n = 0; n < C1_OCH; n++) {
		for (ox = 0; ox < P1_OSIZE; ox++) {
		  for (oy = 0; oy < P1_OSIZE; oy++) {
			max = -256.0;
			for (kx = 0; kx < P1_K; kx++) {
			  for (ky = 0; ky < P1_K; ky++) {
				 tmp = input[n][ox*stride+kx][oy*stride+ky];
				//tmp = *(input+och*isize*isize+krow*isize+kcol+(orow*isize*stride+ocol*stride));
				if (max < tmp) max = tmp;
			  }
			}
			output[n][ox][oy] = max;
			//*(output+och*osize*osize+osize*orow+ocol) = max;
		  }
		}
	  }
	return;
}

void conv2_part(
		float input[C2_ICH][C2_ISIZE][C2_ISIZE],
		float weight[C2_OCH][C2_ICH][C2_K][C2_K],
		float bias[C2_OCH],
		float buffer[MAX_BUF_SIZE],
		ap_uint<16> idd
) {
#pragma HLS INLINE off
	int ox, oy, kx, ky, n, m;
	int i = 0;
	static int stride = 1;
		for (ox = 0 ; ox < C2_OSIZE; ox++) {
			if ((ox < idd * CONV2_LOOPEXE) || ((idd+1) * CONV2_LOOPEXE <= ox)) continue;
			for (oy = 0; oy < C2_OSIZE; oy++) {
				for (n = 0; n < C2_OCH; n++) {
					buffer[i] = bias[n];
					for (m = 0; m < C2_ICH; m++) {
						for (kx = 0; kx < C2_K; kx++) {
							for (ky = 0; ky < C2_K; ky++) {
								buffer[i] += weight[n][m][kx][ky] * input[m][stride*ox+kx][stride*oy+ky];
							}
						}
					}
					i++;
				}
			}
		}
	return;
}

void conv2_r(
	int sendnum,
	float v[MBD][MAX_BUF_SIZE],//recvdata
	ap_fixed<169,69> input[],
	ap_fixed<169,69> tmpin[],
	ap_uint<16> idd,
	float conv2_out[C2_OCH][C2_OSIZE][C2_OSIZE],
	float buffer[MAX_BUF_SIZE]
) {
	int board;
	int ox, oy, n, num;
	int inputnum = CONV1_PKT_SIZE*(MBD-1);
	data_recv(sendnum, inputnum,v, idd, tmpin, input); //data receive
	//arrange results into conv2_out
	for (board = 0; board < MBD; board++) {
		num = 0;
		for (ox = 0 ; ox < C2_OSIZE; ox++) {
			if ((ox < board * CONV2_LOOPEXE) || ((board+1) * CONV2_LOOPEXE <= ox)) continue;
			for (oy = 0; oy < C2_OSIZE; oy++) {
				for (n = 0; n < C2_OCH; n++) {
					if (board == idd) conv2_out[n][ox][oy] = buffer[num];
					else conv2_out[n][ox][oy] = v[board][num];
					num++;
				}
			}
		}
	}
	return;
}

void conv2_all(
		float input[C2_ICH][C2_ISIZE][C2_ISIZE],
		float weight[C2_OCH][C2_ICH][C2_K][C2_K],
		float bias[C2_OCH],
		float conv1_out[C2_OCH][C2_OSIZE][C2_OSIZE],
		ap_uint<16> idd,
		float buffer[MAX_BUF_SIZE],
	 	float v[MBD][MAX_BUF_SIZE],
	 	ap_fixed<169,69> tmpout[MAX_PKT_SIZE],
	 	ap_fixed<169,69> tmpin[MAX_PKT_SIZE*(MBD-1)],
		ap_fixed<169,69> tdata[],
		ap_fixed<169,69> rdata[]
) {
	 int j, k;
	 int sendnum = CONV2_PKT_SIZE;
	 for (j = 0; j < MAX_BUF_SIZE; j++) buffer[j] = 0;
	 for (j = 0; j < MBD; j++)
		 for(k = 0; k < MAX_BUF_SIZE; k++) v[j][k] = 0;
	conv2_part(input, weight, bias, buffer, idd);
	data_trans(sendnum, buffer, idd, tmpout, tdata);
	conv2_r(sendnum, v, rdata, tmpin, idd, conv1_out, buffer);
	int i;
	return;
}

void conv2(
		float input[C2_ICH][C2_ISIZE][C2_ISIZE],
		float weight[C2_OCH][C2_ICH][C2_K][C2_K],
		float bias[C2_OCH],
		float output[C2_OCH][C2_OSIZE][C2_OSIZE]
){
#pragma HLS INLINE off
	int ox, oy, kx, ky, n, m;
	static int stride = 1;
	//Calculate
		for (ox = 0; ox < C2_OSIZE; ox++) {
			for (oy = 0; oy < C2_OSIZE; oy++) {
				for (n = 0; n < C2_OCH; n++) {
					output[n][ox][oy] = bias[n];
					for (m = 0; m < C2_ICH; m++) {
						for (kx = 0; kx < C2_K; kx++) {
							for (ky = 0; ky < C2_K; ky++) {
								output[n][ox][oy] +=
										weight[n][m][kx][ky] *
										input[m][stride*ox+kx][stride*oy+ky];

							}
						}
					}
				}
			}
		}
	return;
}

void pool2(
		float input[C2_OCH][C2_OSIZE][C2_OSIZE],
		float output[C2_OCH][P2_OSIZE][P2_OSIZE]
		){
#pragma HLS INLINE off
	int ox, oy, kx, ky, ix, iy, n, m;
	float tmp, max;

	int stride = 2;
	  for (n = 0; n < C2_OCH; n++) {
		for (ox = 0; ox < P2_OSIZE; ox++) {
		  for (oy = 0; oy < P2_OSIZE; oy++) {
			max = -256.0;
			for (kx = 0; kx < P2_K; kx++) {
			  for (ky = 0; ky < P2_K; ky++) {
				 tmp = input[n][ox*stride+kx][oy*stride+ky];
				//tmp = *(input+och*isize*isize+krow*isize+kcol+(orow*isize*stride+ocol*stride));
				if (max < tmp) max = tmp;
			  }
			}
			output[n][ox][oy] = max;
			//*(output+och*osize*osize+osize*orow+ocol) = max;
		  }
		}
	  }
	return;
}

void flatten(float input[C2_OCH][P2_OSIZE][P2_OSIZE], float output[F1_M]){
#pragma HLS INLINE off
	int ox, oy, n;
	for (n = 0; n < C2_OCH; n++) {
		for (ox = 0; ox < P2_OSIZE; ox++) {
		 	for (oy = 0; oy < P2_OSIZE; oy++) {
		 		output[n*P2_OSIZE*P2_OSIZE + ox*P2_OSIZE + oy] = input[n][ox][oy];
		 	}
		}
	}
	return;
}

void fc1_r(
	int sendnum,
	float v[MBD][MAX_BUF_SIZE],//recvdata
	ap_fixed<169,69> input[],
	ap_fixed<169,69> tmpin[],
	ap_uint<16> idd,
	float fc1_out[F1_N],
	float buffer[MAX_BUF_SIZE]
) {
	int board;
	int ox, oy, n, num, i;
	int inputnum = (CONV1_PKT_SIZE + CONV2_PKT_SIZE)*(MBD-1);
	data_recv(sendnum, inputnum, v, idd, tmpin, input); //data receive
	//arrange results into fc1_out
	for (board = 0; board < MBD; board++) {
		num = 0;
		for (i = 0; i < F1_N; i++) {
			if ((i < board * F1_LOOPEXE) || ((board+1) * F1_LOOPEXE <= i)) continue;
			if (board == idd) fc1_out[i] = buffer[num];
			else fc1_out[i] = v[board][num];
			num++;
		}
	}
	return;
}

void fc1_part(float input[F1_M], float weight[F1_N][F1_M], float bias[F1_N], float buffer[MAX_BUF_SIZE], ap_uint<16> idd
) {
#pragma HLS INLINE off
	int i, j, p;
	int num = 0;
	for (i = 0; i < F1_N; i++) {
		if ((i < idd * F1_LOOPEXE) || ((idd+1) * F1_LOOPEXE <= i)) continue;
		buffer[num] = bias[i];
		for (j = 0; j < F1_M; j+=F1_P) {
			for (p = 0; p < F1_P; p++) {
				#pragma HLS UNROLL
				buffer[num] += input[j+p] * weight[i][j+p];
			}
		}
		if (buffer[num] < 0.0) buffer[num] = 0.0;
		num++;
	}
	return;
}
void fc1_all(
		float input[F1_M],
		float weight[F1_N][F1_M],
		float bias[F1_N],
		float fc1_out[F1_N],
		ap_uint<16> idd,
		float buffer[MAX_BUF_SIZE],
	 	float v[MBD][MAX_BUF_SIZE],
	 	ap_fixed<169,69> tmpout[MAX_PKT_SIZE],
	 	ap_fixed<169,69> tmpin[MAX_PKT_SIZE*(MBD-1)],
		ap_fixed<169,69> tdata[],
		ap_fixed<169,69> rdata[]
) {
	 int j, k;
	 int sendnum = F1_PKT_SIZE;
	 for (j = 0; j < MAX_BUF_SIZE; j++) buffer[j] = 0;
	 for (j = 0; j < MBD; j++)
		 for(k = 0; k < MAX_BUF_SIZE; k++) v[j][k] = 0;
	fc1_part(input, weight, bias, buffer, idd);
	data_trans(sendnum, buffer, idd, tmpout, tdata);
	fc1_r(sendnum, v, rdata, tmpin, idd, fc1_out, buffer);
	int i;
	return;
}

void fc1(float input[F1_M], float weight[F1_N][F1_M], float bias[F1_N], float output[F1_N]) {
#pragma HLS INLINE off
	int i, j, p;
	for (i = 0; i < F1_N; i++) {
		output[i] = bias[i];
		for (j = 0; j < F1_M; j+=F1_P) {
			for (p = 0; p < F1_P; p++) {
#pragma HLS UNROLL
				output[i] += input[j+p] * weight[i][j+p];
			}
		}
		if (output[i] < 0.0) output[i] = 0.0;
	}
	return;
}

void fc2_r(
	int sendnum,
	float v[MBD][MAX_BUF_SIZE],//recvdata
	ap_fixed<169,69> input[],
	ap_fixed<169,69> tmpin[],
	ap_uint<16> idd,
	float fc2_out[F2_N],
	float buffer[MAX_BUF_SIZE]
) {
	int board;
	int ox, oy, n, num, i;
	int inputnum = (CONV1_PKT_SIZE+CONV2_PKT_SIZE+F1_PKT_SIZE)*(MBD-1);
	data_recv(sendnum, inputnum, v, idd, tmpin, input); //data receive
	//arrange results into fc2_out
	for (board = 0; board < MBD; board++) {
		num = 0;
		for (i = 0; i < F1_N; i++) {
			if ((i < board * F2_LOOPEXE) || ((board+1) * F2_LOOPEXE <= i)) continue;
			if (board == idd) fc2_out[i] = buffer[num];
			else fc2_out[i] = v[board][num];
			num++;
		}
	}
	return;
}

void fc2_part(float input[F2_M], float weight[F2_N][F2_M], float bias[F2_N], float buffer[MAX_BUF_SIZE], ap_uint<16> idd
) {
#pragma HLS INLINE off
	int i, j, p;
	int num = 0;
	for (i = 0; i < F2_N; i++) {
		if ((i < idd * F2_LOOPEXE) || ((idd+1) * F2_LOOPEXE <= i)) continue;
		buffer[num] = bias[i];
		for (j = 0; j < F2_M; j+=F2_P) {
			for (p = 0; p < F2_P; p++) {
				#pragma HLS UNROLL
				buffer[num] += input[j+p] * weight[i][j+p];
			}
		}
		if (buffer[num] < 0.0) buffer[num] = 0.0;
		num++;
	}
	return;
}
void fc2_all(
		float input[F2_M],
		float weight[F2_N][F2_M],
		float bias[F2_N],
		float fc2_out[F2_N],
		ap_uint<16> idd,
		float buffer[MAX_BUF_SIZE],
	 	float v[MBD][MAX_BUF_SIZE],
	 	ap_fixed<169,69> tmpout[MAX_PKT_SIZE],
	 	ap_fixed<169,69> tmpin[MAX_PKT_SIZE*(MBD-1)],
		ap_fixed<169,69> tdata[],
		ap_fixed<169,69> rdata[]
) {
	 int j, k, l;
	 int sendnum = F2_PKT_SIZE;
	 float sum = 0.0;
	 float output_tmp[F2_N];
	 for (j = 0; j < MAX_BUF_SIZE; j++) buffer[j] = 0;
	 for (j = 0; j < MBD; j++)
		 for(k = 0; k < MAX_BUF_SIZE; k++) v[j][k] = 0;
	fc2_part(input, weight, bias, buffer, idd);
	data_trans(sendnum, buffer, idd, tmpout, tdata);
	fc2_r(sendnum, v, rdata, tmpin, idd, output_tmp, buffer);
	for (k = 0; k < F2_N; k++) {
		sum += expf(output_tmp[k]);
	}
	for (l = 0; l < F2_N; l++) {
		fc2_out[l] = expf(output_tmp[l]) / sum;
	}
	int i;
	return;
}

void fc2(float input[F2_M], float weight[F2_N][F2_M], float bias[F2_N], float output[F2_N]) {
#pragma HLS INLINE off
	int i, j, k, l, p;
	float sum = 0.0;
	float output_tmp[F2_N];

	for (i = 0; i < F2_N; i++) {
		output_tmp[i] = bias[i];
		for (j = 0; j < F2_M; j+=F2_P) {
			for (p = 0; p < F2_P; p++) {
#pragma HLS UNROLL
				output_tmp[i] += input[j+p] * weight[i][j+p];
			}
		}
	}
	for (k = 0; k < F2_N; k++) {
		sum += expf(output_tmp[k]);
	}
	for (l = 0; l < F2_N; l++) {
		output[l] = expf(output_tmp[l]) / sum;
	}
	return;
}

void store_output(float input[F2_N], float output[RESULTSIZE]){
#pragma HLS INLINE off
	int i;
	for(i = 0; i < F2_N; i++)
		output[i] = input[i];
	return;
}

void lenetall(
		float input[C1_ICH*C1_ISIZE*C1_ISIZE],
		float output[RESULTSIZE],
		float wb[ALL_WB_SIZE],
		ap_fixed<169,69> buf1[],
		ap_fixed<169,69> sw1out[],
		ap_fixed<169,69> sw2in[1],
		ap_fixed<169,69> sw2out[1],
		char id, // 8bit
		ap_uint<4> startt[1],
	    ap_uint<4> stopt[1]
	){
//#pragma for flow
#pragma HLS INTERFACE axis port=startt
#pragma HLS INTERFACE axis port=stopt
#pragma HLS INTERFACE axis port=buf1
#pragma HLS INTERFACE axis port=sw1out
#pragma HLS INTERFACE axis port=sw2in
#pragma HLS INTERFACE axis port=sw2out
#pragma HLS INTERFACE axis port=id
//#pragma HLS dataflow
#pragma HLS INTERFACE axis register both port=input
#pragma HLS INTERFACE axis register both port=output
#pragma HLS INTERFACE axis register both port=wb
//#pragma HLS ARRAY_PARTITION variable=input cyclic factor=2 //in mushak program
//#pragma HLS ARRAY_PARTITION variable=output cyclic factor=2
	//vairables
	ap_uint<16> idd, pid;
	ap_fixed<169,69> sync;
	//static float input_tmp[C1_ICH][C1_ISIZE][C1_ISIZE];
	static float image[C1_ICH][C1_ISIZE][C1_ISIZE];
	static float conv1_w[C1_OCH][C1_ICH][C1_K][C1_K];
	static float conv1_b[C1_OCH];
	static float conv1_out[C1_OCH][C1_OSIZE][C1_OSIZE];
	static float pool1_out[C1_OCH][P1_OSIZE][P1_OSIZE];
	static float conv2_w[C2_OCH][C2_ICH][C2_K][C2_K];
	static float conv2_b[C2_OCH];
	static float conv2_out[C2_OCH][C2_OSIZE][C2_OSIZE];
	static float pool2_out[C2_OCH][P2_OSIZE][P2_OSIZE];
	static float flat_out[F1_M];
	static float fc1_w[F1_N][F1_M];
	static float fc1_b[F1_N];
	static float fc1_out[F1_N];
	static float fc2_w[F2_N][F2_M];
	static float fc2_b[F2_N];
	static float fc2_out[F2_N];
	//buffers
	 float buffer[MAX_BUF_SIZE];
	 float v[MBD][MAX_BUF_SIZE];
	 ap_fixed<169,69> tmpout[MAX_PKT_SIZE];
	 ap_fixed<169,69> tmpin[MAX_PKT_SIZE*(MBD-1)];

	static int wb_flag = 0;
		//Initialize

	if (wb_flag == 0) {
		load_wb(wb, conv1_w, conv1_b, conv2_w, conv2_b, fc1_w, fc1_b, fc2_w, fc2_b);
		wb_flag = 1;
	}
	load_input(input, image);
	startt[0] = 1; //timer start
	idd = id;
	if (idd==0) sw2out[0] = 1;
	else sync= sw2in[0];
	//conv1(image, conv1_w, conv1_b, conv1_out);
	conv1_all(image, conv1_w, conv1_b, conv1_out, idd, buffer, v, tmpout, tmpin, sw1out, buf1);
	//store_output_debug(conv1_out, output);
	pool1(conv1_out, pool1_out);
	//conv2(pool1_out, conv2_w, conv2_b, conv2_out);
	conv2_all(pool1_out, conv2_w, conv2_b, conv2_out, idd, buffer, v, tmpout, tmpin, sw1out, buf1);
	pool2(conv2_out, pool2_out);
	flatten(pool2_out, flat_out);
	//fc1(flat_out, fc1_w, fc1_b, fc1_out);
	fc1_all(flat_out, fc1_w, fc1_b, fc1_out, idd, buffer, v, tmpout, tmpin, sw1out, buf1);
	//fc2(fc1_out, fc2_w, fc2_b, fc2_out);
	fc2_all(flat_out, fc2_w, fc2_b, fc2_out, idd, buffer, v, tmpout, tmpin, sw1out, buf1);
	store_output(fc2_out, output);
	stopt[0] = 1;//timer stop
	return;
}
