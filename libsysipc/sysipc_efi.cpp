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
#include "sysipc.h"
#include "sysipc_efi.h"
#include "EfiCompressDecompress.h"

#include <string.h>
#include <stdio.h>

namespace sysipc
{


//--------------------------------------------------------------------------------------------
void efi_compressor::compress( istream& strm )
{
    uint nbytes = 0;
    vector<byte>    in_buff;
    while ( strm.good()  )
    {
        in_buff.resize( in_buff.size() + 1024 * 64 );
        strm.read( (char *)&in_buff[ nbytes ], in_buff.size() - nbytes );
		nbytes += strm.gcount();
    }
    if ( nbytes > 0 )
        compress( (char *)&in_buff[ 0 ], nbytes );
}

//--------------------------------------------------------------------------------------------
bool efi_compressor::compress( istream& strm, ostream& ostrm )
{
    if ( strm.peek() == EOF )    // zero bytes? 
        return false;

    int status = EfiCompress( strm, ostrm );
    return (status == EFI_SUCCESS);
}

//--------------------------------------------------------------------------------------------
bool efi_compressor::compress( const char* src, uint src_size, ostream& ostrm  )
{
    bool rc = false;
    if ( src && src_size > 0 )
    {
        int status = EfiCompress( (UINT8 *)src, src_size, ostrm );
        rc = (status == EFI_SUCCESS);
    }
    return rc;
}

//--------------------------------------------------------------------------------------------
void efi_compressor::compress( const char* src, uint src_size )
{
    if ( src > 0 )
    {
        // start with 64k
        buffer.resize( 1024 * 64 );
        UINT32_EFI  dest_size = buffer.size();
        EFI_STATUS status = EfiCompress( (byte*)src, src_size, &buffer[ 0 ], &dest_size );
        if ( status == EFI_BUFFER_TOO_SMALL )
        {
            buffer.resize( dest_size );
            status = EfiCompress( (byte*)src, src_size, &buffer[ 0 ], &dest_size );
        }
        if ( status == EFI_SUCCESS )
        {
            buffer.resize( dest_size );
        }
        else
            buffer.clear();
    }
}

//--------------------------------------------------------------------------------------------
void efi_compressor::write( ostream& strm )
{
    if ( buffer.size() > 0 )
    {
        strm.write( (const char* )&buffer[ 0 ], buffer.size() );  
        strm.flush();
    }
}

//--------------------------------------------------------------------------------------------
bool efi_decompressor::decompress( istream& strm )
{
    uint nbytes = 0;
    bool rtn = false;

    // read in 5 bytes and see if it is xml
    vector<byte>    in_buff( 56);
    in_buff[ 5 ] = 0;
    if ( strm.good()  )
    {
        strm.read( (char *)&in_buff[ 0 ], 5 );
        nbytes = strm.gcount();
    }

    if ( nbytes == 5 )
    {
        if ( 0 == strcmp( (const char*) &in_buff.front(), "<?xml" ) )
        {
            // this is uncompressed xml
            buffer = in_buff;
            while ( strm.good()  )
            {
                buffer.resize( buffer.size() + 1024 * 64 );
                strm.read( (char *)&buffer[ nbytes ], buffer.size() - nbytes );
                nbytes += strm.gcount();
            }
            buffer.resize( nbytes );
			rtn = true;
        }
        else
        {
            // this could be compressed xml
            while ( strm.good()  )
            {
                in_buff.resize( in_buff.size() + 1024 * 64 );
                strm.read( (char *)&in_buff[ nbytes ], in_buff.size() - nbytes );
                nbytes += strm.gcount();
            }
            in_buff.resize( nbytes );
            if ( in_buff.size() > 8 )
            {
                UINT32_EFI  dest_size = 0;
                UINT32_EFI  scratch_size = 0;
    
                EFI_STATUS status = EfiGetInfo( (char*)&in_buff[ 0 ], in_buff.size(), &dest_size, &scratch_size );
                if ( ( status == EFI_SUCCESS ) && ( dest_size > 0 ) )
                {
                    if ( dest_size <= MAX_DECOMPRESSED_FILE_SIZE )
                    {
                        buffer.resize( dest_size );
                        vector<byte>	scratch_buff( scratch_size );
                        status = EfiDecompress( &in_buff[ 0 ], in_buff.size(), 
                                            &buffer[ 0 ], buffer.size(), 
                                            &scratch_buff[  0 ], scratch_buff.size() );
                
                        if ( status == EFI_SUCCESS )
                            rtn = true;
                    }
                    else
                    {
                        cerr << "dest_size > MAX_DECOMPRESSED_FILE_SIZE" << endl;
                    }
                }
            }
        }
    }
    if ( !rtn )
        buffer.clear();

    return rtn;
}

//--------------------------------------------------------------------------------------------
bool efi_decompressor::decompress( istream& strm, ostream& ostrm  )
{
    uint nbytes = 0;
    bool rtn = false;

    // read in 5 bytes and see if it is xml
    vector<byte>    in_buff( 56);
    in_buff[ 5 ] = 0;
    if ( strm.good()  )
    {
        strm.read( (char *)&in_buff[ 0 ], 5 );
        nbytes = strm.gcount();
    }

    if ( nbytes == 5 )
    {
        if ( 0 == strcmp( (const char*) &in_buff.front(), "<?xml" ) )
        {
            // this is uncompressed xml
            strm.seekg(0);
            ostrm << strm.rdbuf() ;
            if ( ostrm )
                rtn = true;
            else
                cerr << "error while copying\n" ;
        }
        else
        {
            // this could be compressed xml
            UINT32_EFI  dest_size = 0;
            UINT32_EFI  scratch_size = 0;
    
            EFI_STATUS status = EfiGetInfo( strm, &dest_size, &scratch_size );
            if ( ( status == EFI_SUCCESS ) && ( dest_size > 0 ) )
            {
                if ( dest_size <= MAX_DECOMPRESSED_FILE_SIZE )
                {
                    vector<byte>	scratch_buff( scratch_size );
                    status = EfiDecompress( strm, ostrm,
                                            &scratch_buff[  0 ], scratch_buff.size() );
                
                    if ( status == EFI_SUCCESS )
                        rtn = true;
                }
                else
                {
                    cerr << "dest_size > MAX_DECOMPRESSED_FILE_SIZE" << endl;
                }
            }
        }
    }

    return rtn;
}

//--------------------------------------------------------------------------------------------
void efi_decompressor::write( ostream& strm )
{
    if ( buffer.size() > 0 )
        strm.write( (const char* )&buffer[ 0 ], buffer.size() );  
}


}

