#include "shared_ptr.h"
#include <vector>
#include <set>
#include <iostream>
#include <algorithm>

struct Foo
{ 
	Foo( int _x ) : x(_x) {}
	~Foo() { std::cout << "Destructing a Foo with x=" << x << "\n"; }
	int x;
	/* ... */
};

typedef woodycxx::smart_prt::shared_ptr<Foo> FooPtr;

struct FooPtrOps
{
	bool operator()( const FooPtr & a, const FooPtr & b )
	{ return a->x > b->x; }
	void operator()( const FooPtr & a )
	{ std::cout << a->x << "\n"; }
};

int test_main()
{
	std::vector<FooPtr>         foo_vector;
	std::set<FooPtr,FooPtrOps>  foo_set; // NOT multiset!

	FooPtr foo_ptr( new Foo( 2 ) );
	foo_vector.push_back( foo_ptr );
	foo_set.insert( foo_ptr );

	foo_ptr.reset( new Foo( 1 ) );
	foo_vector.push_back( foo_ptr );
	foo_set.insert( foo_ptr );

	foo_ptr.reset( new Foo( 3 ) );
	foo_vector.push_back( foo_ptr );
	foo_set.insert( foo_ptr );

	foo_ptr.reset ( new Foo( 2 ) );
	foo_vector.push_back( foo_ptr );
	foo_set.insert( foo_ptr );

	std::cout << "foo_vector:\n";
	std::for_each( foo_vector.begin(), foo_vector.end(), FooPtrOps() );

	std::cout << "\nfoo_set:\n"; 
	std::for_each( foo_set.begin(), foo_set.end(), FooPtrOps() );
	std::cout << "\n";

	//  Expected output:
	//
	//   foo_vector:
	//   2
	//   1
	//   3
	//   2
	//   
	//   foo_set:
	//   3
	//   2
	//   1
	//
	//   Destructing a Foo with x=2
	//   Destructing a Foo with x=1
	//   Destructing a Foo with x=3
	//   Destructing a Foo with x=2

	return 0;
}

#if 1
int main()
{
	test_main();
	return 0;
}

#endif