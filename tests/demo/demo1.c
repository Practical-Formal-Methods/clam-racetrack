// RUN: %clam -O0 --crab-dom=zones  --crab-check=assert --crab-sanity-checks "%s" 2>&1 | OutputCheck -l debug %s
// CHECK: ^2  Number of total safe checks$
// CHECK: ^0  Number of total error checks$
// CHECK: ^0  Number of total warning checks$


extern void __CRAB_assume (int);
extern void __CRAB_assert(int);
extern int  __CRAB_nd(void);

int main() {
  int k = __CRAB_nd();
  int n = __CRAB_nd();
  __CRAB_assume (k > 0);
  __CRAB_assume (n > 0);
  
  int x = k;
  int y = k;
  while (x < n) {
    x++;
    y++;
  }
  __CRAB_assert (x >= y);
  __CRAB_assert (x <= y);  
  return 0;
}
