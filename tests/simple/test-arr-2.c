// RUN: %clam -O0 --lower-unsigned-icmp --crab-dom=int --crab-track=arr --crab-heap-analysis=llvm-dsa --crab-check=assert --crab-sanity-checks "%s" 2>&1 | OutputCheck -l debug %s
// RUN: %clam -O0 --lower-unsigned-icmp --crab-dom=int --crab-track=arr --crab-heap-analysis=ci-sea-dsa --crab-check=assert --crab-sanity-checks "%s" 2>&1 | OutputCheck -l debug %s
// RUN: %clam -O0 --lower-unsigned-icmp --crab-dom=int --crab-track=arr --crab-heap-analysis=cs-sea-dsa --crab-check=assert --crab-sanity-checks "%s" 2>&1 | OutputCheck -l debug %s
// CHECK: ^1  Number of total safe checks$
// CHECK: ^0  Number of total error checks$
// CHECK: ^0  Number of total warning checks$

extern int nd (void);
extern void __CRAB_assert(int);

int x = 5;
int y = 3;

int a[10];

int main ()
{
  int i;
  int *p =&x;
  *p= *p + 1;
  for (i=0;i<10;i++)
  {
    if (nd ())
      a[i] =y;
    else 
      a[i] =x;
  }
  y++;
  int res = a[i-1];

  __CRAB_assert(res >= 0 && res <= 6);
  return res;
}
