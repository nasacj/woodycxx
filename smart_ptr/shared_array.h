/*
 *  shared_array.h
 *
 *  Created on: 2014-10-15
 *      Author: qianchj
 */

#ifndef WOODYCXX_SMART_PTR_SHARED_ARRAY_H_
#define WOODYCXX_SMART_PTR_SHARED_ARRAY_H_

#include "shared_count.h"
#include "checked_delete.h"

namespace woodycxx { namespace smart_prt {

	template<class T> class shared_array
	{
	private:
		typedef checked_array_deleter<T> deleter;
		typedef shared_array<T> this_type;

	public:

		typedef T element_type;

		shared_array() : px( 0 ), pn()
		{
		}

		template<class Y>
		explicit shared_array( Y * p ): px( p ), pn( p, checked_array_deleter<Y>() )
		{
		}

		shared_array( shared_array const & r ) : px( r.px ), pn( r.pn )
		{
		}

		shared_array & operator=( shared_array const & r )
		{
			this_type( r ).swap( *this );
			return *this;
		}

		void reset() BOOST_NOEXCEPT
		{
			this_type().swap( *this );
		}

		template<class Y> void reset( Y * p ) // Y must be complete
		{
			BOOST_ASSERT( p == 0 || p != px ); // catch self-reset errors
			this_type( p ).swap( *this );
		}

		T & operator[] (std::ptrdiff_t i) const
		{
			BOOST_ASSERT(px != 0);
			BOOST_ASSERT(i >= 0);
			return px[i];
		}

		T * get() const
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

		void swap(shared_array<T> & other)
		{
			std::swap(px, other.px);
			pn.swap(other.pn);
		}

	private:

		template<class Y> friend class shared_array;

		T * px;                     // contained pointer
		detail::shared_count pn;    // reference counter

	};  // shared_array

	template<class T> inline bool operator==(shared_array<T> const & a, shared_array<T> const & b)
	{
		return a.get() == b.get();
	}

	template<class T> inline bool operator!=(shared_array<T> const & a, shared_array<T> const & b)
	{
		return a.get() != b.get();
	}

}}

#endif