
;; Function main (main)

main ()
{
  int i;
  int x[3];
  int b[3];
  int a[3];
  int D.1993;
  int D.1992;
  int D.1990;
  int D.1987;
  int D.1986;

  a[0] = 1;
  a[1] = 2;
  a[2] = 3;
  b[0] = 2;
  b[1] = 3;
  b[2] = 2;
  i = 0;
  goto <D.1983>;
  <D.1982>:
  D.1986 = a[i];
  D.1987 = b[i];
  if (D.1986 < D.1987) goto <D.1988>; else goto <D.1989>;
  <D.1988>:
  D.1990 = a[i];
  x[i] = D.1990;
  goto <D.1991>;
  <D.1989>:
  D.1992 = b[i];
  x[i] = D.1992;
  <D.1991>:
  i = i + 1;
  <D.1983>:
  if (i <= 2) goto <D.1982>; else goto <D.1984>;
  <D.1984>:
  D.1993 = 0;
  goto <D.1994>;
  <D.1994>:
  return D.1993;
}

