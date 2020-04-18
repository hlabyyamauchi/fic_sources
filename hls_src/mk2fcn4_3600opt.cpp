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

void load_input(fix input[ISIZE], fix buffer[IPSIZE][IDEPTH]) ;
void store_output(fix buffer[OPSIZE][ODEPTH], fix output[OSIZE]);
void load_wb(fix wb[1], fix buffer[1]);
void fcn(fix input[IPSIZE][IDEPTH], fix output[OPSIZE][ODEPTH], fix wb[1]);
void pe(fix input_reg[IPSIZE], fix weight_reg[OPSIZE][IPSIZE], fix output_read[OPSIZE], fix output_write[OPSIZE]);

//------TOP FUNCTION------
void fcn4bdopt2(fix input[ISIZE], fix output[OSIZE], fix wb[1],
		ap_fixed<169,69> buf1[MAX_PKT_SIZE*(MBD-1)],
		ap_fixed<169,69> sw1out[MAX_PKT_SIZE],
		ap_fixed<169,69> sw2in[1],
		ap_fixed<169,69> sw2out[1],
		char id, // 8bit
		float startt[1],
	    float stopt[1]) {
//#pragma HLS ARRAY_PARTITION variable=input cyclic factor=1 dim=1
//#pragma HLS ARRAY_PARTITION variable=output cyclic factor=1 dim=1
#pragma HLS INTERFACE axis register both port=input
#pragma HLS INTERFACE axis register both port=output
#pragma HLS INTERFACE axis register both port=wb
#pragma HLS INTERFACE axis port=startt
#pragma HLS INTERFACE axis port=stopt
#pragma HLS INTERFACE axis port=buf1
#pragma HLS INTERFACE axis port=sw1out
#pragma HLS INTERFACE axis port=sw2in
#pragma HLS INTERFACE axis port=sw2out
#pragma HLS INTERFACE axis port=id
//#pragma HLS DATAFLOW

// INPUT AND OUTPUT BUFFERS
	static fix input_buffer[IPSIZE][IDEPTH];
#pragma HLS RESOURCE variable=input_buffer core=RAM_2P_LUTRAM
#pragma HLS ARRAY_PARTITION variable=input_buffer complete dim=1
	static fix output_buffer[OPSIZE][ODEPTH];
#pragma HLS RESOURCE variable=output_buffer core=RAM_2P_LUTRAM
#pragma HLS ARRAY_PARTITION variable=output_buffer complete dim=1

	static fix wb_buffer[1];
ap_uint<8> idd;
//MAIN DATAFLOW
	idd = id;
	idd &= (MBD-1);
		ap_fixed<169,69> sync;
		if (idd==0) sw2out[0] = 1;
		else sync= sw2in[0];
		static int startt_flag = 0;
		if (startt_flag == 0) {
			startt[0] = 1;
			startt_flag = 1;
		}
	load_wb(wb, wb_buffer);
	load_input(input, input_buffer);
	fcn(input_buffer, output_buffer, wb_buffer);
//	store_output(output_buffer, output);
//packing and send
	ap_fixed<169,69> tmpout[MAX_PKT_SIZE];
		////////
	ap_fixed<169,69> packet = 0;
	ap_uint<16> head;
	int i, j, k, l;
	i = 0;
	head = idd;
	for (i = 0; i < OPSIZE; i++){
		for(j = 0, l = 0; j < ODEPTH; j+=8, l++){
			#pragma HLS PIPELINE II=1
			packet(168,153) = head(15,0);
			packet(127,112) = ((ap_fixed<16,4>)output_buffer[i][j])(15,0);
			packet(111,96) =((ap_fixed<16,4>)output_buffer[i][j+1])(15,0);
			packet(95,80) = ((ap_fixed<16,4>)output_buffer[i][j+2])(15,0);
			packet(79,64) =((ap_fixed<16,4>)output_buffer[i][j+3])(15,0);
			packet(63,48) =((ap_fixed<16,4>)output_buffer[i][j+4])(15,0);
			packet(47,32) =((ap_fixed<16,4>)output_buffer[i][j+5])(15,0);
			packet(31,16) =((ap_fixed<16,4>)output_buffer[i][j+6])(15,0);
			packet(15,0) =((ap_fixed<16,4>)output_buffer[i][j+7])(15,0);
			tmpout[i*1+l] = packet;
		}
	}
	for (k = 0; k < MAX_PKT_SIZE; k++) sw1out[k] = tmpout[k];
	//recv
	ap_fixed<169,69> tmpin[MAX_PKT_SIZE*(MBD-1)];
	for(i=0; i<MAX_PKT_SIZE*(MBD-1); i++) tmpin[i] = buf1[i]; //receive data
	static int stop_flag = 0;
	if (stop_flag == 0) {
		stopt[0] = 1;
		stop_flag = 1;
	}
	store_output(output_buffer, output);
	return;
}

void load_input(fix input[ISIZE], fix buffer[IPSIZE][IDEPTH]) {
	int ip, id;
	static int flag = 0;
	if (flag < ALL_WB) {
		flag++;
	} else {
		for (id = 0; id < IDEPTH; id++) {
			for (ip = 0; ip < IPSIZE; ip++) {
#pragma HLS PIPELINE
				buffer[ip][id] = input[id*IPSIZE+ip];
			}
		}
	}
}

void load_wb(fix wb[1], fix buffer[1]) {
	static int flag = 0;
	if (flag < ALL_WB) {
		buffer[0] = wb[0];
		flag++;
	}
}

void store_output(fix buffer[OPSIZE][ODEPTH], fix output[OSIZE]) {
	int op, od;
	static int flag = 0;
	if (flag < ALL_WB) {
		flag++;
	} else {
		for (od = 0; od < ODEPTH; od++) {
#pragma HLS PIPELINE
			for (op = 0; op < OPSIZE; op++) {
#pragma HLS UNROLL
				output[od*IPSIZE+op] = buffer[op][od];
			}
		}
	}
}

void fcn(fix input[IPSIZE][IDEPTH], fix output[OPSIZE][ODEPTH], fix wb[1]) {
#pragma HLS ARRAY_PARTITION variable=input complete dim=1
#pragma HLS ARRAY_PARTITION variable=output complete dim=1

	int i, o, ore, owr, wo, wi, cnt;

	static int flag = 0;

	static int s_op=0, s_ip=0, s_od=0, s_id=0;
	int op, ip, od, id;

	//WEIGHT_BUFFER
	static fix w_buffer[OPSIZE][IPSIZE][ODEPTH][IDEPTH];
#pragma HLS ARRAY_PARTITION variable=w_buffer complete dim=1
#pragma HLS ARRAY_PARTITION variable=w_buffer complete dim=2

	//BIAS_BUFFER
	static fix b_buffer[OPSIZE][ODEPTH];
#pragma HLS ARRAY_PARTITION variable=b_buffer complete dim=1

	//OUTPUT_TMP
	fix output_tmp[OPSIZE][ODEPTH];
#pragma HLS RESOURCE variable=output_tmp core=RAM_2P_LUTRAM
#pragma HLS ARRAY_PARTITION variable=output_tmp complete dim=1

	//REGs for PEs
	fix input_reg[IPSIZE];
#pragma HLS ARRAY_PARTITION variable=input_reg complete dim=0
	fix weight_reg[OPSIZE][IPSIZE];
#pragma HLS ARRAY_PARTITION variable=weight_reg complete dim=0
	fix output_read[OPSIZE];
#pragma HLS ARRAY_PARTITION variable=output_read complete dim=0
	fix output_write[OPSIZE];
#pragma HLS ARRAY_PARTITION variable=output_write complete dim=0


	if (flag < ALL_WB){
		if (flag < OSIZE*ISIZE) {
			w_buffer[s_op][s_ip][s_od][s_id] = wb[0];
			s_ip++;
			if (s_ip > IPSIZE) {s_id++; s_ip=0;}
			if (s_id > IDEPTH) {s_op++; s_id=0;}
			if (s_op > OPSIZE) {s_od++; s_op=0;}
			if (s_od > ODEPTH) {s_od = 0;}
		}else  {
			cnt = flag-OSIZE*ISIZE;
			s_op = cnt%OPSIZE;
			s_od = cnt/OPSIZE;
			b_buffer[s_op][s_od] = wb[0];
		}
		flag++;
	} else {

// INIT OUTPUT_TMP by BIAS_BUFFER
		for (od = 0; od < ODEPTH; od++) {
#pragma HLS PIPELINE II=1
			for (op = 0; op < OPSIZE; op++) {
#pragma HLS UNROLL
				output_tmp[op][od] = b_buffer[op][od];
			}
		}

//MAIN of FCN
		for (id = 0; id < IDEPTH; id++) {
			for (od = 0; od < ODEPTH; od++) {
#pragma HLS PIPELINE II=1
				//NAKAMI
				for (ip = 0; ip < IPSIZE; ip++) {
#pragma HLS UNROLL
					input_reg[ip] = input[ip][id];
				}

				for (op = 0; op < OPSIZE; op++) {
#pragma HLS UNROLL
					for (ip = 0; ip < IPSIZE; ip++) {
#pragma HLS UNROLL
						weight_reg[op][ip] = w_buffer[op][ip][od][id];
					}
				}

				for (op = 0; op < OPSIZE; op++) {
#pragma HLS UNROLL
					output_read[op] = output_tmp[op][od];
				}

				pe(	input_reg,weight_reg,output_read,output_write);

				for (op = 0; op < OPSIZE; op++) {
#pragma HLS UNROLL
					output_tmp[op][od] = output_write[op];
				}
				//NAKAMI END
			}
		}

// STORE OUTPUT_TMP to OUTPUT_BUFFER
		for (od = 0; od < ODEPTH; od++) {
#pragma HLS PIPELINE II=1
			for (op = 0; op < OPSIZE; op++) {
#pragma HLS UNROLL
				output[op][od] = output_tmp[op][od];
			}
		}
	}
}


void pe(
		fix input_reg[IPSIZE],
		fix weight_reg[OPSIZE][IPSIZE],
		fix output_read[OPSIZE],
		fix output_write[OPSIZE]
		) {
#pragma HLS ARRAY_PARTITION variable=input_reg complete dim=0
#pragma HLS ARRAY_PARTITION variable=weight_reg complete dim=0
#pragma HLS ARRAY_PARTITION variable=output_read complete dim=0
#pragma HLS ARRAY_PARTITION variable=output_write complete dim=0

	int ip, op, ia, oa, ore, owr;

	fix output_tmp[OPSIZE];
#pragma HLS ARRAY_PARTITION variable=output_tmp complete dim=0

//#pragma HLS PIPELINE II=1

	//READ_OUTPUT_REG
	for (ore = 0; ore < OPSIZE; ore++) {
		#pragma HLS UNROLL
		output_tmp[ore] = output_read[ore];
	}

	//MULT_ADD
	for (op = 0; op < OPSIZE; op++) {
		#pragma HLS UNROLL
		for (ip = 0; ip < IPSIZE; ip++) {
			#pragma HLS UNROLL
			output_tmp[op] += input_reg[ip] * weight_reg[op][ip];
		}
	}

	//WRITE_OUTPUT_REG
	for (owr = 0; owr < OPSIZE; owr++) {
		#pragma HLS UNROLL
		output_write[owr] = output_tmp[owr];
	}


}
