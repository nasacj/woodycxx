/*
 * FileDescriptor.h
 *
 *  Created on: 2014-12-16
 *      Author: qianchj
 */

#ifndef FILE_DESCRIPTOR_H_
#define FILE_DESCRIPTOR_H_

#include <base/Types.h>

namespace woodycxx { namespace io {

typedef int Handle;

class FileDescriptor
{
private:
    Handle handler;

public:
    FileDescriptor() {}
    ~FileDescriptor(){}
    explicit FileDescriptor(Handle handle_n) : handler(handle_n) {}
    Handle getHandler() { return this->handler; }
    void set( Handle handler_ot) { this->handler = handler_ot; }
};

}}

#endif
