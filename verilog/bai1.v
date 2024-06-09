module bai1(a, b, c, Out);
  output Out;
  input a, b, c;
  wire na, nb, nc;
  wire y1, y2, y3;

  not n1(na, a);
  not n2(nb, b);
  not n3(nc, c);

  and a1(y1, a, c);
  and a2(y2, nb, c);
  and a3(y3, na, b, nc);

  or o1(Out, y1, y2, y3);
endmodule

