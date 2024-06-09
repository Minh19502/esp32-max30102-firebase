`timescale 1ns/1ns

module abc_tb;
   reg x,y;
   wire out;

   xor_gate dut(.a(x), .b(y), .out(out)); // Corrected instantiation

   initial begin
      x=0; y=0;
      #10;
      x=1; y=0;
      #10;
      x=0; y=1;
      #10;
      x=1; y=1;
      #10;
      $finish;
   end
endmodule

