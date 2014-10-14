/*
 *  sp_counted_impl.h
 *
 *  Created on: 2014-10-14
 *      Author: qianchj
 */

#ifndef WOODYCXX_SMART_PTR_SP_COUNTED_IMPL_H_
#define WOODYCXX_SMART_PTR_SP_COUNTED_IMPL_H_

#include <iostream>
using namespace std;

namespace woodycxx { namespace smart_prt { namespace detail {

	template<class X> class sp_counted_impl_p: public sp_counted_base
	{
	private:

		X * px_;

		sp_counted_impl_p( sp_counted_impl_p const & );
		sp_counted_impl_p & operator= ( sp_counted_impl_p const & );

		typedef sp_counted_impl_p<X> this_type;

	public:

		explicit sp_counted_impl_p( X * px ): px_( px )
		{

		}

		virtual void dispose() // nothrow
		{
			cout << "sp_counted_impl_p::dispose()" << endl;
			delete px_;
		}

	};

}}}

#endif