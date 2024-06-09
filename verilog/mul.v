module mul(in1, in2, clk, out);
  input [15:0] in1, in2;
  input clk;
  output reg [31:0] out;
  
  always @(posedge clk)
   begin
	out = in1 * in2;
   end
endmodule 