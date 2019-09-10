/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#include "shared_array.h"
#include <iostream>

using namespace std;

struct Foo {
  Foo() : x(0) {}
  Foo(int _x) : x(_x) {}
  ~Foo() { std::cout << "Destructing a Foo with x=" << x << "\n"; }
  int x;
  /* ... */
};

typedef woodycxx::smart_prt::shared_array<Foo> FooArray;

void test() {
  FooArray(new Foo[10]);
}

int main() {
  test();
  return 0;
}

