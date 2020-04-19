module wrap_0 (
    input          clk,
    input          rst_n,
    input          ap_rst,
    input          ap_start,
    input [7:0]    id,
(* keep = "TRUE" *)	output   startt_valid,
(* keep = "TRUE" *)	output   stopt_valid,
	input [63:0]    slotreg,

    input          rasin_valid,
    input [15:0]    rasin_data,
    output         rasin_ready, // Output

    input          sw0out_ready,
    output         sw0out_valid,
    output [168:0] sw0out_data,
    input          sw0in_valid,
    input [168:0]  sw0in_data,
    output         sw0in_ready,

(* keep = "TRUE" *)    input          sw1out_ready,
 (* keep = "TRUE" *)   output         sw1out_valid,
(* keep = "TRUE" *)    output [168:0] sw1out_data,
(* keep = "TRUE" *)    input          sw1in_valid,
 (* keep = "TRUE" *)   input [168:0]  sw1in_data,
 (* keep = "TRUE" *)   output         sw1in_ready,

(* keep = "TRUE" *)    input          sw2out_ready,
(* keep = "TRUE" *)    output         sw2out_valid,
(* keep = "TRUE" *)    output [168:0] sw2out_data,
(* keep = "TRUE" *)    input          sw2in_valid,
(* keep = "TRUE" *)    input [168:0]  sw2in_data,
(* keep = "TRUE" *)    output         sw2in_ready,

    input          sw3out_ready,
    output         sw3out_valid,
    output [168:0] sw3out_data,
    input          sw3in_valid,
    input [168:0]  sw3in_data,
    output         sw3in_ready,

    output         rasout_valid,
    input          rasout_ready, // input
    output [3:0]   rasout_data
);

(* keep = "TRUE" *) wire done0;
(* keep = "TRUE" *) wire [168:0] buf1_data, buf2_data;
(* keep = "TRUE" *) wire buf1_valid, buf1_ready, buf2_valid, buf2_ready;

(* keep = "TRUE" *) wire [31:0] dataresult, dataimage, datawb, data01, data02;
(* keep = "TRUE" *) wire validresult, validimage, validwb, valid01, valid02;
(* keep = "TRUE" *) wire readyresult, readyimage, readywb, ready01, ready02;

assign sw0out_data = 0;
assign sw3out_data = 0;

assign sw0out_valid = 0;
assign sw3out_valid = 0;

assign sw0in_ready = 0;
assign sw3in_ready = 0;

lenetall16_0 lenetall (
    .ap_clk(clk),
    .ap_rst_n(!ap_rst),
    .ap_start(ap_start),
    .ap_done(done0),
    .ap_idle(),
    .ap_ready(),
    .input_r_TVALID(validimage),
    .input_r_TREADY(readyimage),
    .input_r_TDATA(dataimage),
    .output_r_TVALID(validresult),
    .output_r_TREADY(readyresult),
    .output_r_TDATA(dataresult),
    .wb_TVALID(validwb),
    .wb_TREADY(readywb),
    .wb_TDATA(datawb),
    .buf1_V_TVALID(buf1_valid),
    .buf1_V_TREADY(buf1_ready),
    .buf1_V_TDATA(buf1_data),
    .sw1out_V_TVALID(sw1out_valid),
    .sw1out_V_TREADY(sw1out_ready),
    .sw1out_V_TDATA(sw1out_data),
    .sw2in_V_TVALID(sw2in_valid),
    .sw2in_V_TREADY(sw2in_ready),
    .sw2in_V_TDATA(sw2in_data),
    .sw2out_V_TVALID(sw2out_valid),
    .sw2out_V_TREADY(sw2out_ready),
    .sw2out_V_TDATA(sw2out_data),
    .id_TVALID(1),
    .id_TREADY(),
    .id_TDATA(id),
    .startt_TVALID(startt_valid),
    .startt_TREADY(1),
    .startt_TDATA(),
    .stopt_TVALID(stopt_valid),
    .stopt_TREADY(1),
    .stopt_TDATA()
  );

  buf_lenetall_16_0  buf_lenetall_3(
      .ap_clk(clk),
      .ap_rst_n(!ap_rst),
      .ap_start(ap_start),
      .ap_done(),
      .ap_idle(),
      .ap_ready(),
      .sw1in_V_TVALID(sw1in_valid),
      .sw1in_V_TREADY(sw1in_ready),
      .sw1in_V_TDATA(sw1in_data),
      .buf1_V_TVALID(buf1_valid),
      .buf1_V_TREADY(buf1_ready),
      .buf1_V_TDATA(buf1_data)
    );
    
lenetall_io_0 lenetall_io (
    .ap_clk(clk),
    .ap_rst_n(!ap_rst),
    .ap_start(ap_start),
    .ap_done(),
    .ap_idle(),
    .ap_ready(),
    .rasin_TVALID(valid01),
    .rasin_TREADY(ready01),
    .rasin_TDATA(data01),
    .rasout_TVALID(valid02),
    .rasout_TREADY(ready02),
    .rasout_TDATA(data02),
    .image_r_TVALID(validimage),
    .image_r_TREADY(readyimage),
    .image_r_TDATA(dataimage),
    .wb_TVALID(validwb),
    .wb_TREADY(readywb),
    .wb_TDATA(datawb),
    .result_TVALID(validresult),
    .result_TREADY(readyresult),
    .result_TDATA(dataresult)
  );

/*
lenetinput_0 lenetinput(
    .ap_clk(clk),
    .ap_rst_n(!ap_rst),
    .ap_start(ap_start),
    .ap_done(),
    .ap_idle(),
    .ap_ready(),
    .image_r_TVALID(validimage),
    .image_r_TREADY(readyimage),
    .image_r_TDATA(dataimage),
    .wb_TVALID(validwb),
    .wb_TREADY(readywb),
    .wb_TDATA(datawb)
);
*/

rasfic_tx32_0 tx32 (
    .ap_clk(clk),
    .ap_rst_n(!ap_rst),
    .ap_start(ap_start),
    .ap_done(),
    .ap_idle(),
    .ap_ready(),
    
    .output_r_TVALID(rasout_valid),
    .output_r_TREADY(rasout_ready),
    .output_r_TDATA(rasout_data),
    .input_r_TVALID(validresult),
    .input_r_TREADY(readyresult),
    .input_r_TDATA(dataresult)
);

rasfic_rx32_0 rx32 (
    .ap_clk(clk),
    .ap_rst_n(!ap_rst),
    .ap_start(ap_start),
    .ap_done(),
    .ap_idle(),
    .ap_ready(),
    
    .input_r_TVALID(rasin_valid),
    .input_r_TDATA(rasin_data),
    .input_r_TREADY(rasin_ready),

    .output_r_TDATA(data01),
    .output_r_TVALID(valid01),
    .output_r_TREADY(ready01)
);

endmodule
