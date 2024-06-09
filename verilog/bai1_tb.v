module bai1_tb;

  // Inputs
  reg a, b, c;
  // Output
  wire Out;

  // Instantiate the module bai1
  bai1 DUT(
    .a(a),
    .b(b),
    .c(c),
    .Out(Out)
  );

  // Stimulus
  initial begin
    // Test case 1
    a = 0; b = 0; c = 1;
    #10; // wait for 10 time units
    // Test case 2
    a = 1; b = 0; c = 1;
    #10; 
    // Test case 3
    a = 0; b = 1; c = 1;
    #10;
    // Test case 4
    a = 1; b = 1; c = 1;
    #10;
    // Test case 5
    a = 0; b = 0; c = 0;
    #10;
    // Test case 6
    a = 1; b = 0; c = 0;
    #10;
    // Test case 7
    a = 0; b = 1; c = 0;
    #10;
    // Test case 8
    a = 1; b = 1; c = 0;
    #10;
    // End of simulation
    $finish;
  end

endmodule

