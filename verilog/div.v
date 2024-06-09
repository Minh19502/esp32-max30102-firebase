module div(in1 ,in2, clk, out);
  input [31:0] in1;
  input [7:0] in2;
  input clk;
  output reg [31:0] out;

  always @(posedge clk)
   begin
	out = in1 / in2;
   end 
endmodule
