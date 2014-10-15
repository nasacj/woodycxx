/*
 *  shared_ptr.h
 *
 *  Created on: 2014-10-11
 *      Author: qianchj
 */

#ifndef WOODYCXX_SMART_PTR_SHARED_COUNT_H_
#define WOODYCXX_SMART_PTR_SHARED_COUNT_H_

#include "sp_counted_base.h"
#include "sp_counted_impl.h"
#include <functional>       // std::less
#include <assert.h>

namespace woodycxx { namespace smart_prt { namespace detail{

class shared_count
{
private:
	sp_counted_base * pi_;

public:
	shared_count():pi_(0)
	{
	}

	template<class Y> explicit shared_count( Y * p ): pi_( 0 )
	{
		pi_ = new sp_counted_impl_p<Y>( p );
		assert(0 != pi_);
	}

	template<class P, class D> shared_count( P * p, D d ): pi_(0)
	{
		pi_ = new sp_counted_impl_pd<P, D>(p, d);
		assert(0 != pi_);
	}

	shared_count(shared_count const & r): pi_(r.pi_)
	{
		if( pi_ != 0 ) pi_->add_ref_copy();
	}

	 ~shared_count()
	 {
		 if( pi_ != 0 ) pi_->release();
	 }

	 shared_count & operator= (shared_count const & r)
	 {
		 sp_counted_base * tmp = r.pi_;

		 if( tmp != pi_ )
		 {
			 if( tmp != 0 ) tmp->add_ref_copy();
			 if( pi_ != 0 ) pi_->release();
			 pi_ = tmp;
		 }

		 return *this;
	 }

	 void swap(shared_count & r) // nothrow
	 {
		 sp_counted_base * tmp = r.pi_;
		 r.pi_ = pi_;
		 pi_ = tmp;
	 }

	 long use_count() const // nothrow
	 {
		 return pi_ != 0 ? pi_->use_count() : 0;
	 }

	 bool unique() const // nothrow
	 {
		 return use_count() == 1;
	 }

	 bool empty() const // nothrow
	 {
		 return pi_ == 0;
	 }

	 friend inline bool operator==(shared_count const & a, shared_count const & b)
	 {
		 return a.pi_ == b.pi_;
	 }

	 friend inline bool operator<(shared_count const & a, shared_count const & b)
	 {
		 return std::less<sp_counted_base *>()( a.pi_, b.pi_ );
	 }


};

}}}
#endif