/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
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