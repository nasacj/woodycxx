/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#ifndef WOODYCXX_SMART_PTR_SP_COUNTED_IMPL_H_
#define WOODYCXX_SMART_PTR_SP_COUNTED_IMPL_H_

#include "../checked_delete.h"

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
			woodycxx::smart_prt::checked_delete(px_);
		}

	};

	template<class P, class D> class sp_counted_impl_pd: public sp_counted_base
	{
	private:

		P ptr; // copy constructor must not throw
		D del; // copy constructor must not throw

		sp_counted_impl_pd( sp_counted_impl_pd const & );
		sp_counted_impl_pd & operator= ( sp_counted_impl_pd const & );

		typedef sp_counted_impl_pd<P, D> this_type;

	public:

		// pre: d(p) must not throw
		sp_counted_impl_pd( P p, D & d ): ptr( p ), del( d )
		{
		}

		sp_counted_impl_pd( P p ): ptr( p ), del()
		{
		}

		virtual void dispose() // nothrow
		{
			del( ptr );
		}

	};

}}}

#endif