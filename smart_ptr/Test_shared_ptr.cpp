/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#include "shared_ptr.h"
#include <vector>
#include <set>
#include <iostream>
#include <algorithm>

struct Foo
{
public:
	explicit Foo( int _x ) : x(_x) { std::cout << "Foo(" << _x << ")" << std::endl; }
    Foo(const Foo & foo) : x(foo.x){std::cout << "Copy Foo()" << std::endl;}
    Foo & operator=(Foo const & foo){std::cout << "Foo::=" << std::endl; return *this;}
	~Foo() { std::cout << "Destructing a Foo with x=" << x << "\n"; }
	int x;
    virtual void fun(){std::cout << "Foo::fun()" << std::endl;}
	/* ... */
};

class Foo_child : public Foo
{
public:
    Foo_child( int _x ) : Foo(_x) { std::cout << "Foo_child(" << _x << ")" << std::endl; }
    Foo_child(const Foo_child & foo) : Foo(foo){std::cout << "Copy Foo_child()" << std::endl;}
    Foo_child & operator=(Foo_child const & foo){std::cout << "Foo_child::=" << std::endl; return *this;}
    ~Foo_child() { std::cout << "Destructing a Foo_child with x=" << x << "\n"; }
    virtual void fun(){std::cout << "Foo_child::fun()" << std::endl;}
};

class Conta
{
public:
    Foo foo;
    Conta(int _x):foo(_x){}
    Foo& get_foo()
    {
        Foo& f = foo;
        return f;
    }
};

typedef woodycxx::smart_prt::shared_ptr<Foo> FooPtr;
typedef woodycxx::smart_prt::shared_ptr<Foo_child> Foo_childPtr;

struct FooPtrOps
{
	bool operator()( const FooPtr & a, const FooPtr & b )
	{ return a->x > b->x; }
	void operator()( const FooPtr & a )
	{ std::cout << a->x << "\n"; }
};

Foo ret_foo()
{
    Foo foo(5);
    return foo;
}

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

void test_return_ref()
{
    std::cout << "--------- y --------" << std::endl;
    Conta conta(8);
    Foo& foo_c = conta.get_foo();
    foo_c.fun();
    conta.get_foo().fun();

    std::cout << "---------" << std::endl;
    Foo foo_c2(1);
    foo_c2 = conta.get_foo();

    std::cout << "---------" << std::endl;
    Foo foo_c3 = foo_c;
}

int main()
{
    Foo foo = ret_foo();
    std::cout << "foo.x=" << foo.x << std::endl;
    
    std::cout << "--------- x --------" << std::endl;
    FooPtr fooptr(new Foo(6));
    Foo_childPtr foo_childptr(new Foo_child(7));
    //fooptr = foo_childptr;
    //foo_childptr->fun();

    test_return_ref();

    std::cout << "--------- test_main --------" << std::endl;
	test_main();
	return 0;
}
