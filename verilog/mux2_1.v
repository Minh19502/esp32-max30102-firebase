module Mux_2_1(
	input in_0, // Mux first input
	input in_1, // Mux Second input
	input sel, // Select input
	output mux_out // Mux output
);

assign mux_out = sel ? in_1 : in_0;

endmodule

