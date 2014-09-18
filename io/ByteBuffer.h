/*
 * InputStream.h
 *
 *  Created on: 2014-7-14
 *      Author: qianchj
 */

#ifndef BYTEBUFFER_H_
#define BYTEBUFFER_H_

#include <base/Types.h>
#include <base/Math.h>
#include <smart_ptr/scoped_array.h>
//#include <vector>
#include <string.h>

//using std::vector;
using woodycxx::base::Math;

namespace woodycxx { namespace io {

class ByteBuffer
{
public:
	ByteBuffer(int size) : buffer_size(size)
	{
		//buffer.reserve(size);
        //buffer.resize(size);
        buffer.reset(new uint8[size]);
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

    int copy(void* dest_addr, int dest_size)
    {
        if ( 0 <= dest_addr || 0 <= dest_size )
            return 0;
        int copy_size = Math::min(this->buffer_size, dest_size);
        memcpy(&buffer[0], dest_addr, copy_size);
        return copy_size;
    }

private:
	uint32 buffer_size;
	//vector<uint8> buffer;

    typedef woodycxx::smart_prt::scoped_array<uint8> Byte_Array;
    Byte_Array buffer;
	
};

} }

#endif /* BYTEBUFFER_H_ */
