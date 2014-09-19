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

    /* Not allow to copy in = way, use @copy function
    ByteBuffer & operator=(ByteBuffer const & buf)
    {
        buffer.reset(buf.buffer.get());
        this->buffer_size = buf.buffer_size;
        return *this;
    }*/
    /*
    ByteBuffer& operator=(const ByteBuffer& b)
    {
        if (this != &b)
        {
            this->buffer = b.buffer;
        }
        return (*this);
    }*/

	int getSize() { return this->buffer_size; }

	uint8& operator[](int n){ return buffer[n]; }

    

    int copy(void* dest_addr, int dest_size)
    {
        if ( 0 == dest_addr || dest_size <= 0 )
            return 0;
        int copy_size = Math::min(this->buffer_size, dest_size);
        memcpy(dest_addr, &buffer[0], copy_size);
        return copy_size;
    }

    int copy(ByteBuffer& dest_buf)
    {
        if (&dest_buf != this)
        {
            dest_buf.reset(this->buffer_size);
            return this->copy(dest_buf.buffer.get(), this->buffer_size);
        }
        return 0;
    }

    void reset(int size)
    {
        if ( size <= 0 )
        {
            return;
        }
        buffer.reset(new uint8[size]);
        this->buffer_size = size;
    }

private:
	uint32 buffer_size;
	//vector<uint8> buffer;

    typedef woodycxx::smart_prt::scoped_array<uint8> Byte_Array;
    Byte_Array buffer;
	
};

} }

#endif /* BYTEBUFFER_H_ */
