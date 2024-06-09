module and3_1_tb();
reg a,b,c;
wire out;

and3_1 U0(.out(out),.a(a),.b(b),.c(c));
initial
begin
a=0;
b=0;
c=0;
end
always
begin
#1 a=1;
#1 b=1;
#1 c=1;
#1 a=0;
#1 b=0;
#1 c=0;
#10 $finish;
end
endmodule:and3_1_tb
