// RUN: %clam -O0 --crab-inter --crab-dom=int --crab-track=arr --crab-check=assert --crab-sanity-checks "%s" 2>&1 | OutputCheck %s
// CHECK: ^0  Number of total safe checks$
// CHECK: ^0  Number of total error checks$
// CHECK: ^1  Number of total warning checks$

extern void __CRAB_assert(int);

int rec1 (int x);
int rec2 (int y);

int foo (int x) {
  int y = x +1;
  return y + 2;
}

int bar (int a) {
  int x = a;
  int w = 5;
  return foo (x);
}


int rec1 (int x) {
  if (x == 0) return 0;
  else return rec2 (x-1);
}

int rec2 (int x) {
  return rec1 (x);
}

int main (){
  int x = 3;
  int y = bar (x);
  int z = rec1 (y);
  int w= foo (y);
  __CRAB_assert (w == 10);
  return z + w;
}
