extern int nd ();

int foo (int x, int flag) {

  if (flag > 0)
    return x + flag;
  else
    return x + 2;
}

int main (){

  int x = 5;
  int res = foo (x, nd ());
  return res;
}
