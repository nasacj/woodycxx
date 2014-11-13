/****************************************************************************
 *
 *	IBM Software.
 *	(c) Copyright International Business Machines Corp. 2004.
 *	All rights reserved.
 *
 *	U.S. Government	Users Restricted Rights.
 *
 *	Use, duplication or disclosure restricted
 *	by GSA ADP Schedule Contract with IBM Corp.
 *
 *	Licensed Material - Property of	IBM.
 *
 ****************************************************************************
 *
 *   sysipc_handle.h
 *
 ***************************************************************************/
#ifndef _SYSIPC_HANDLE_H
#define _SYSIPC_HANDLE_H

#include "sysipc.h"

namespace sysipc {

/*--------------------------------------------------------------------------
 * struct Trep
 *------------------------------------------------------------------------*/
template <class T>
struct Trep
{
   uint  refcount;
   T*    data;
   Trep( T* p ) : refcount( 1 ), data( p ){}
   ~Trep(){ delete data; }
};

/*--------------------------------------------------------------------------
 * class Thandle
 *------------------------------------------------------------------------*/
template < class T >
class Thandle
{

    Trep<T>*    rep;
protected:
    void  bind( T* p ){ rep = new Trep<T>( p ); }
public:
    Thandle( const Thandle<T>& h ) : rep( h.rep ){ rep->refcount++; }
   Thandle( T* p ) : rep( new Trep<T>( p ) ){}
   Thandle() : rep( 0 ){}
   ~Thandle();
   Thandle<T>& operator=( const Thandle<T>& h );
   Thandle<T>& operator=( const T* );

   const T* operator->() const { return rep->data; }
   T* operator->() { return rep->data; }

   T* get_rep(){ return rep->data; }
   const T* get_rep() const { return rep->data; }
   bool has_rep() const { return rep != 0 ; }
};

/*--------------------------------------------------------------------------
 * operator==( Thandle<T>& hl, Thandle<T>& hr ) 
 *------------------------------------------------------------------------*/
template < class T >
inline bool operator==( const Thandle<T>& h1, const Thandle<T>& h2 )
{
   return h1.has_rep() && h2.has_rep() && ( h1.get_rep() == h2.get_rep() ); 
}

/*--------------------------------------------------------------------------
 * operator<( Thandle<T>& hl, Thandle<T>& hr ) 
 *------------------------------------------------------------------------*/
template < class T >
inline bool operator<( const Thandle<T>& h1, const Thandle<T>& h2 )
{
   return h1.has_rep() && h2.has_rep() && ( *h1.get_rep() < *h2.get_rep() ); 
}


/*--------------------------------------------------------------------------
 * Thandle::~Thandle
 *------------------------------------------------------------------------*/
template <class T>
Thandle<T>::~Thandle()
{
   if ( rep && ( --rep->refcount == 0 ) )
   {
      delete rep;
      rep = 0;
   }
}

/*--------------------------------------------------------------------------
 * operator=
 *------------------------------------------------------------------------*/
template <class T>
Thandle<T>& Thandle<T>::operator=( const Thandle<T>& h )
{
   h.rep->refcount++;
   if ( rep && ( --rep->refcount == 0 ) )
      delete rep;

   rep = h.rep;
   return *this;
}

/*--------------------------------------------------------------------------
 * operator=
 *------------------------------------------------------------------------*/
template <class T>
Thandle<T>& Thandle<T>::operator=( const T* p )
{
   if ( rep && ( --rep->refcount == 0 ) )
      delete rep;

   rep = new Trep<T>( (T*) p );
   return *this;
}

}

#endif
