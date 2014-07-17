/*
 * InputStream.h
 *
 *  Created on: 2014-7-14
 *      Author: qianchj
 */

#ifndef INPUTSTREAM_H_
#define INPUTSTREAM_H_

#include "Types.h"
#include "ByteBuffer.h"

namespace woodycxx { namespace io {

enum IO_ERROR
{
	BUFFER_OUT_OF_SIZE = 0xF0000001
};

class IO_ERROR_CODE
{
public:
	static const int NullPointerError   = 0xF00000001;
	static const int InvalidPrameter    = 0xF00000002;
};

/*
* This abstract class is the superclass of all classes representing an input stream of bytes.
* Applications that need to define a subclass of InputStream must always provide a method that returns the next byte of input.
*/
class InputStream
{
public:
	
    /**
     * Reads the next byte of data from the input stream. The value byte is
     * returned as an <code>int</code> in the range <code>0</code> to
     * <code>255</code>. If no byte is available because the end of the stream
     * has been reached, the value <code>-1</code> is returned. This method
     * blocks until input data is available, the end of the stream is detected,
     * or an exception is thrown.
     *
     * <p> A subclass must provide an implementation of this method.
     *
     * @return     the next byte of data, or <code>-1</code> if the end of the
     *             stream is reached.
     */
    virtual int read() = 0;

    /*
    Reads some number of bytes from the input stream and stores them into the buffer array b. The number of bytes actually read is returned as an integer. This method blocks until input data is available, end of file is detected, or an exception is thrown.
    If the length of b is zero, then no bytes are read and 0 is returned; otherwise, there is an attempt to read at least one byte. If no byte is available because the stream is at the end of the file, the value -1 is returned; otherwise, at least one byte is read and stored into b.
    The first byte read is stored into element b[0], the next one into b[1], and so on. The number of bytes read is, at most, equal to the length of b. Let k be the number of bytes actually read; these bytes will be stored in elements b[0] through b[k-1], leaving elements b[k] through b[b.length-1] unaffected.
    The read(b) method for class InputStream has the same effect as:

        read(b, 0, b.getSize())

    Parameters:
    b the buffer into which the data is read.

    Returns:
    the total number of bytes read into the buffer, or -1 is there is no more data because the end of the stream has been reached.
    */
	virtual int read(ByteBuffer& b);

    /*
    Reads up to len bytes of data from the input stream into an array of bytes. An attempt is made to read as many as len bytes, but a smaller number may be read. The number of bytes actually read is returned as an integer.
    This method blocks until input data is available, end of file is detected, or an exception is thrown.
    If len is zero, then no bytes are read and 0 is returned; otherwise, there is an attempt to read at least one byte. If no byte is available because the stream is at end of file, the value -1 is returned; otherwise, at least one byte is read and stored into b.
    The first byte read is stored into element b[off], the next one into b[off+1], and so on. The number of bytes read is, at most, equal to len. Let k be the number of bytes actually read; these bytes will be stored in elements b[off] through b[off+k-1], leaving elements b[off+k] through b[off+len-1] unaffected.
    In every case, elements b[0] through b[off] and elements b[off+len] through b[b.length-1] are unaffected.
    The read(b, off, len) method for class InputStream simply calls the method read() repeatedly. If the first such call results in an IOException, that exception is returned from the call to the read(b, off, len) method. If any subsequent call to read() results in a IOException, the exception is caught and treated as if it were end of file; the bytes read up to that point are stored into b and the number of bytes read before the exception occurred is returned. The default implementation of this method blocks until the requested amount of input data len has been read, end of file is detected, or an exception is thrown. Subclasses are encouraged to provide a more efficient implementation of this method.
    
    Parameters:
    b the buffer into which the data is read.
    off the start offset in array b at which the data is written.
    len the maximum number of bytes to read.
    
    Returns:
    the total number of bytes read into the buffer, or -1 if there is no more data because the end of the stream has been reached.
    */
	virtual int read(ByteBuffer& b, int off, int len);


    /*
    Returns an estimate of the number of bytes that can be read (or skipped over) from this input stream without blocking by the next invocation of a method for this input stream. The next invocation might be the same thread or another thread. A single read or skip of this many bytes will not block, but may read or skip fewer bytes.
    Note that while some implementations of InputStream will return the total number of bytes in the stream, many will not. It is never correct to use the return value of this method to allocate a buffer intended to hold all data in this stream.
    A subclass' implementation of this method may choose to retrun ERROR if this input stream has been closed by invoking the close() method.
    The available method for class InputStream always returns 0.
    
    This method should be overridden by subclasses.
    
    Returns:
    an estimate of the number of bytes that can be read (or skipped over) from this input stream without blocking or 0 when it reaches the end of the input stream.
    */
	virtual int available();


    virtual ~InputStream(){}
};



}}


#endif /* INPUTSTREAM_H_ */

