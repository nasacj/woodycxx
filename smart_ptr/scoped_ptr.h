/*
 * scoped_ptr.h
 *
 *  Created on: 2014-9-16
 *      Author: qianchj
 */

#ifndef WOODYCXX_SMART_PTR_SCOPED_PTR_H_
#define WOODYCXX_SMART_PTR_SCOPED_PTR_H_

# include <memory>          // for std::auto_ptr
# include <assert.h>

namespace woodycxx { namespace smart_prt {

    //  scoped_ptr mimics a built-in pointer except that it guarantees deletion
    //  of the object pointed to, either on destruction of the scoped_ptr or via
    //  an explicit reset(). scoped_ptr is a simple solution for simple needs;
    //  use shared_ptr or std::auto_ptr if your needs are more complex.

    template<class T> class scoped_ptr // noncopyable
    {
    private:

        T * px;

        scoped_ptr(scoped_ptr const &);
        scoped_ptr & operator=(scoped_ptr const &);

        typedef scoped_ptr<T> this_type;

        void operator==( scoped_ptr const& ) const;
        void operator!=( scoped_ptr const& ) const;

    public:

        typedef T element_type;

        explicit scoped_ptr( T * p = 0 ): px( p ) // never throws
        {
        }

        explicit scoped_ptr( std::auto_ptr<T> p ) : px( p.release() )
        {
        }

        ~scoped_ptr() // never throws
        {
            if (px)
            {
                delete px;
            }
        }

        void reset(T * p = 0) // never throws
        {
            assert( p == 0 || p != px ); // catch self-reset errors
            this_type(p).swap(*this);
        }

        T & operator*() const // never throws
        {
            assert( px != 0 );
            return *px;
        }

        T * operator->() const // never throws
        {
            assert( px != 0 );
            return px;
        }

        T * get() const
        {
            return px;
        }

        void swap(scoped_ptr & b)
        {
            T * tmp = b.px;
            b.px = px;
            px = tmp;
        }
    };

    
}}

#endif
