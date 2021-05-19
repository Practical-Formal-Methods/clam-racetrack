// RUN: %clam -O0 --lower-unsigned-icmp --crab-dom=int --crab-track=arr --crab-heap-analysis=llvm-dsa --crab-check=assert --crab-sanity-checks "%s" 2>&1 | OutputCheck %s
// RUN: %clam -O0 --lower-unsigned-icmp --crab-dom=int --crab-track=arr --crab-heap-analysis=ci-sea-dsa --crab-check=assert --crab-sanity-checks "%s" 2>&1 | OutputCheck %s
// RUN: %clam -O0 --lower-unsigned-icmp --crab-dom=int --crab-track=arr --crab-heap-analysis=cs-sea-dsa --crab-check=assert --crab-sanity-checks "%s" 2>&1 | OutputCheck %s

// CHECK: ^1  Number of total safe checks$
// CHECK: ^0  Number of total error checks$
// CHECK: ^0  Number of total warning checks$
extern int nd ();
extern void __CRAB_assert(int);

int main () {
  // local array
  int a[10];
  int i;
  for (i=0;i<10;i++) {
    if (nd ())
      a[i] =0;
    else 
      a[i] =5;
  }

  int res = a[i-1];
  __CRAB_assert(res >= 0 && res <= 5);
  return res;
}
