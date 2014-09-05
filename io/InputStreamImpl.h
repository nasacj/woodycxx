/*
 * InputStreamImpl.h
 *
 *  Created on: 2014-7-14
 *      Author: qianchj
 */

#ifndef INPUTSTREAMIMPL_H_
#define INPUTSTREAMIMPL_H_

#include "InputStream.h"

namespace woodycxx { namespace io {

class InputStreamImpl : public InputStream
{
public:
    virtual int read();
};


} }

#endif /* INPUTSTREAMIMPL_H_ */