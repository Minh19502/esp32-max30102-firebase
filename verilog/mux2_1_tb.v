`timescale 1ns / 1ps

module mux2_1_tb();

  reg a, b, s;
  wire out;

  // Instantiate the mux
  Mux_2_1 dut(
    .in_0(a),
    .in_1(b),
    .sel(s),
    .mux_out(out)
  );

  // Stimulus generation
  initial begin
    // Test case 1: s=0
    s = 0;
    a = 0; b = 1; // Example input values
    #10; // Delay for observation

    // Test case 2: s=1
    s = 1;
    a = 1; b = 0; // Example input values
    #10; // Delay for observation

    // Add more test cases as needed
  end

endmodule

