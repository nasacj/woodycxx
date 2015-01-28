/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#ifndef WOODYCXX_SMART_PTR_SHARED_PTR_H_
#define WOODYCXX_SMART_PTR_SHARED_PTR_H_

#include "detail/shared_count.h"
#include <algorithm>            // for std::swap
#include <assert.h>

namespace woodycxx { namespace smart_prt {

    namespace detail
    {
        template< class T > struct sp_element
        {
            typedef T type;
        };

        // sp_dereference, return type of operator*
        template< class T > struct sp_dereference
        {
            typedef T & type;
        };

        // sp_member_access, return type of operator->
        template< class T > struct sp_member_access
        {
            typedef T * type;
        };

		template< class Y > inline void sp_pointer_construct( Y * p, woodycxx::smart_prt::detail::shared_count & pn )
		{
			woodycxx::smart_prt::detail::shared_count( p ).swap( pn );
			//woodycxx::smart_prt::shared_ptr::detail::sp_enable_shared_from_this( ppx, p, p );
		}
    }

    //
    //  shared_ptr
    //
    //  An enhanced relative of scoped_ptr with reference counted copy semantics.
    //  The object pointed to is deleted when the last shared_ptr pointing to it
    //  is destroyed or reset.
    //

    template<class T> class shared_ptr
    {
	public:
		typedef typename woodycxx::smart_prt::detail::sp_element< T >::type element_type;
	private:
		typedef shared_ptr<T> this_type;

		element_type * px;              // contained pointer
		woodycxx::smart_prt::detail::shared_count pn;				// reference counter

    public:
        shared_ptr() : px( 0 ), pn()
        {
        }

		template<class Y>
		explicit shared_ptr( Y * p ): px( p ), pn() // Y must be complete
		{
			woodycxx::smart_prt::detail::sp_pointer_construct( p, pn );
		}

        //shared_ptr( std::sp_nullptr_t ) : px( 0 ), pn(){}

		shared_ptr( shared_ptr const & r ) : px( r.px ), pn( r.pn )
		{
		}

		shared_ptr & operator=( shared_ptr const & r )
		{
			this_type(r).swap(*this);
			return *this;
		}

		void swap( shared_ptr & other )
		{
			std::swap(px, other.px);
			pn.swap(other.pn);
		}

		void reset()
		{
			this_type().swap(*this);
		}

		template<class Y> void reset( Y * p ) // Y must be complete
		{
			assert( p == 0 || p != px ); // catch self-reset errors
			this_type( p ).swap( *this );
		}

		typename woodycxx::smart_prt::detail::sp_dereference< T >::type operator* () const
		{
			assert( px != 0 );
			return *px;
		}

		typename woodycxx::smart_prt::detail::sp_member_access< T >::type operator-> () const 
		{
			assert( px != 0 );
			return px;
		}

		element_type * get() const
		{
			return px;
		}

		bool unique() const
		{
			return pn.unique();
		}

		long use_count() const
		{
			return pn.use_count();
		}

	};

	template<class T, class U> inline bool operator==(const shared_ptr<T> & a, const shared_ptr<U> & b)
	{
		return a.get() == b.get();
	}

	template<class T, class U> inline bool operator!=(const shared_ptr<T> & a, const shared_ptr<U> & b)
	{
		return a.get() != b.get();
	}

	template<class T> inline void swap(shared_ptr<T> & a, shared_ptr<T> & b)
	{
		a.swap(b);
	}

	template<class T> inline typename shared_ptr<T>::element_type * get_pointer(shared_ptr<T> const & p)
	{
		return p.get();
	}
}}
#endif
