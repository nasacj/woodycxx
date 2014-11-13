/******************************************************************************
 * 
 *  Licensed Materials - Property of IBM.
 * 
 *  (C) Copyright IBM Corporation 2009
 * 
 *  All Rights Reserved.
 * 
 *  US Government Users Restricted Rights -
 *  Use, duplication or disclosure restricted by
 *  GSA ADP Schedule Contract with IBM Corporation.
 * 
 *****************************************************************************/
#ifndef _SYSIPC_EFI_H
#define _SYSIPC_EFI_H

namespace sysipc
{

//--------------------------------------------------------------------------------------------
class efi_compressor
{
    vector<byte>    buffer;

    // hide
    efi_compressor( const efi_compressor& );
    efi_compressor& operator=( const efi_compressor& );

public:
	efi_compressor(){}
	void compress( istream& strm );
	bool compress( istream& strm, ostream& ostrm );    // compress straight to file
    bool compress( const char* buff, uint size, ostream& ostrm );    // compress straight to file 
    void compress( const char* buff, uint size );
    void write( ostream& strm );
};

//--------------------------------------------------------------------------------------------
class efi_decompressor
{
    /// \brief Largest decopmressed file permitted
    /// 
    /// The decompressor comes up with a huge number if the compressed file is NOT efi
    /// compressed. This prevents it from attempting a huge buffer allocation. 
    static const uint MAX_DECOMPRESSED_FILE_SIZE = 1024 * 1024 * 8;

    vector<byte>    buffer;

    // hide
    efi_decompressor( const efi_decompressor& );
    efi_decompressor& operator=( const efi_decompressor& );

public:
	efi_decompressor(){}
    bool decompress( istream& strm );
    bool decompress( istream& strm, ostream& ostrm );    // decompress straight to file to lower memory use
    void write( ostream& strm );
    const byte* data() const { return &buffer[ 0 ]; }
    uint size() const { return buffer.size(); }
};

}
#endif
