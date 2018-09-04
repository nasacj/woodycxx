#include "LRUCache.h"
#include <iostream>

using namespace woodycxx;

#if 1
int main() {
  LRUCache<int, int> cache(2);
  cout << cache.get(2) << endl;
  cache.set(2, 6);
  //cache.printCache();
  cout << cache.get(1) << endl;
  cache.set(1, 5);
  //cache.printCache();
  cache.set(1, 2);
  //cache.printCache();
  cout << cache.get(1) << endl;
  cout << cache.get(2) << endl;
  return 0;
}
#endif
