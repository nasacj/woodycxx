/*
 * InputStream.h
 *
 *  Created on: 2014-7-14
 *      Author: qianchj
 */

#ifndef BYTEBUFFER_H_
#define BYTEBUFFER_H_

#include "Types.h"
#include <vector>

using std::vector;

namespace woodycxx { namespace io {

class ByteBuffer
{
public:
	ByteBuffer(int size) : buffer_size(size)
	{
		//buffer.reserve(size);
        buffer.resize(size);
	}

	int getSize() { return this->buffer_size; }

	uint8& operator[](int n){ return buffer[n]; }

    /*
    ByteBuffer& operator=(const ByteBuffer& b)
    {
        if (this != &b)
        {
            this->buffer = b.buffer;
        }
        return (*this);
    }*/

private:
	uint32 buffer_size;
	vector<uint8> buffer;
	
};

} }

#endif /* BYTEBUFFER_H_ */