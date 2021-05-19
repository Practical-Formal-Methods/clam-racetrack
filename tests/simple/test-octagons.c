// RUN: %clam -O0 --crab-dom=oct --crab-check=assert --crab-sanity-checks "%s" 2>&1 | OutputCheck %s
// CHECK: ^1  Number of total safe checks$
// CHECK: ^0  Number of total error checks$
// CHECK: ^0  Number of total warning checks$

extern void __CRAB_assert(int);
extern void __SEAHORN_error(int);

int main (){

  int k = 200;
  int n = 100;
  int x = 0, y = k;

  while (x  < n) {
    x++;
    y = k - 2*x;
  }
  __CRAB_assert(x+y <= k);

  return x+y;
}
