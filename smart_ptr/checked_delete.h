/*
 *  checked_delete.h
 *
 *  Created on: 2014-10-15
 *      Author: qianchj
 */

#ifndef WOODYCXX_SMART_PTR_CHECKED_DELETE_H_
#define WOODYCXX_SMART_PTR_CHECKED_DELETE_H_

// verify that types are complete for increased safety

namespace woodycxx { namespace smart_prt {

template<class T> inline void checked_delete(T * x)
{
	// intentionally complex - simplification causes regressions
	typedef char type_must_be_complete[ sizeof(T)? 1: -1 ];
	(void) sizeof(type_must_be_complete);
	delete x;
}

template<class T> inline void checked_array_delete(T * x)
{
	typedef char type_must_be_complete[ sizeof(T)? 1: -1 ];
	(void) sizeof(type_must_be_complete);
	delete [] x;
}

template<class T> struct checked_deleter
{
	typedef void result_type;
	typedef T * argument_type;

	void operator()(T * x) const
	{
		// boost:: disables ADL
		checked_delete(x);
	}
};

template<class T> struct checked_array_deleter
{
	typedef void result_type;
	typedef T * argument_type;

	void operator()(T * x) const
	{
		checked_array_delete(x);
	}
};

}}

#endif