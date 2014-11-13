
/*++

Copyright (c) 2004 - 2006, Intel Corporation                                              
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Compress.h

Abstract:

  Header file for compression routine.
  Providing both EFI and Tiano Compress algorithms.
  
--*/

#ifndef _EFICOMPRESSDECOMPRESS_H_
#define _EFICOMPRESSDECOMPRESS_H_

#include <iostream>

//
// EFI Data Types based on ANSI C integer types in EfiBind.h
//
typedef int intn_t;
typedef unsigned int uintn_t;
#ifdef WIN32
typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef long int32_t;
typedef unsigned long uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;
#define UINT8_MAX 255
#else
#include <stdint.h>
#endif /* WIN32 */

typedef uint8_t BOOLEAN_EFI;
typedef intn_t INTN;
typedef uintn_t UINTN;
#ifndef _BASETSD_H_
typedef int8_t INT8;
#endif
typedef uint8_t UINT8;
typedef int16_t INT16;
typedef uint16_t UINT16;
#ifndef _BASETSD_H_
typedef int32_t INT32_EFI;
typedef uint32_t UINT32_EFI;
#endif
typedef int64_t INT64;
typedef uint64_t UINT64;
typedef uint8_t CHAR8;
typedef uint16_t CHAR16;
typedef UINT64 EFI_LBA;

//
// Modifiers for EFI Data Types used to self document code.
// Please see EFI coding convention for proper usage.
//
#ifndef IN
//
// Some other envirnments use this construct, so #ifndef to prevent
// mulitple definition.
//
#define IN
#define OUT
#define OPTIONAL
#endif
#define UNALIGNED

//
// Modifiers to absract standard types to aid in debug of problems
//
#define CONST     const
#define STATIC    static
#define VOID      void
#define VOLATILE  volatile

//
// Modifier to ensure that all protocol member functions and EFI intrinsics
// use the correct C calling convention. All protocol member functions and
// EFI intrinsics are required to modify thier member functions with EFIAPI.
//
#define _EFIAPI
#define EFIAPI  _EFIAPI

//
// EFI Constants. They may exist in other build structures, so #ifndef them.
//

#ifndef NULL
#define NULL  ((VOID *) 0)
#endif
//
// EFI Data Types derived from other EFI data types.
//
typedef UINTN EFI_STATUS;

//
// Set the upper bit to indicate EFI Error.
//
#define EFI_MAX_BIT               0x80000000
#define EFIERR(a)                 (EFI_MAX_BIT | (a))

#define EFIWARN(a)                (a)
#define EFI_ERROR(a)              (((INTN) (a)) < 0)

#define EFI_SUCCESS               0
#define EFI_LOAD_ERROR            EFIERR (1)
#define EFI_INVALID_PARAMETER     EFIERR (2)
#define EFI_UNSUPPORTED           EFIERR (3)
#define EFI_BAD_BUFFER_SIZE       EFIERR (4)
#define EFI_BUFFER_TOO_SMALL      EFIERR (5)
#define EFI_NOT_READY             EFIERR (6)
#define EFI_DEVICE_ERROR          EFIERR (7)
#define EFI_WRITE_PROTECTED       EFIERR (8)
#define EFI_OUT_OF_RESOURCES      EFIERR (9)
#define EFI_VOLUME_CORRUPTED      EFIERR (10)
#define EFI_VOLUME_FULL           EFIERR (11)
#define EFI_NO_MEDIA              EFIERR (12)
#define EFI_MEDIA_CHANGED         EFIERR (13)
#define EFI_NOT_FOUND             EFIERR (14)
#define EFI_ACCESS_DENIED         EFIERR (15)
#define EFI_NO_RESPONSE           EFIERR (16)
#define EFI_NO_MAPPING            EFIERR (17)
#define EFI_TIMEOUT               EFIERR (18)
#define EFI_NOT_STARTED           EFIERR (19)
#define EFI_ALREADY_STARTED       EFIERR (20)
#define EFI_ABORTED               EFIERR (21)
#define EFI_ICMP_ERROR            EFIERR (22)
#define EFI_TFTP_ERROR            EFIERR (23)
#define EFI_PROTOCOL_ERROR        EFIERR (24)
#define EFI_INCOMPATIBLE_VERSION  EFIERR (25)
#define EFI_SECURITY_VIOLATION    EFIERR (26)
#define EFI_CRC_ERROR             EFIERR (27)

#define EFI_WARN_UNKNOWN_GLYPH    EFIWARN (1)
#define EFI_WARN_DELETE_FAILURE   EFIWARN (2)
#define EFI_WARN_WRITE_FAILURE    EFIWARN (3)
#define EFI_WARN_BUFFER_TOO_SMALL EFIWARN (4)


EFI_STATUS EfiCompress ( IN UINT8 *SrcBuffer, IN UINT32_EFI SrcSize, IN std::ostream& mOstrm ); 
EFI_STATUS EfiCompress ( IN std::istream& mIstrm, IN std::ostream& mOstrm );

/*++

Routine Description:

  Efi compression routine.

--*/
EFI_STATUS
EfiCompress (
  IN      UINT8   *SrcBuffer,
  IN      UINT32_EFI  SrcSize,
  IN      UINT8   *DstBuffer,
  IN OUT  UINT32_EFI  *DstSize
  )
;

/*++

Routine Description:

  The compression routine.

Arguments:

  SrcBuffer   - The buffer storing the source data
  SrcSize     - The size of source data
  DstBuffer   - The buffer to store the compressed data
  DstSize     - On input, the size of DstBuffer; On output,
                the size of the actual compressed data.

Returns:

  EFI_BUFFER_TOO_SMALL  - The DstBuffer is too small. In this case,
                DstSize contains the size needed.
  EFI_SUCCESS           - Compression is successful.
  EFI_OUT_OF_RESOURCES  - No resource to complete function.
  EFI_INVALID_PARAMETER - Parameter supplied is wrong.

--*/
typedef
EFI_STATUS
(*COMPRESS_FUNCTION) (
  IN      UINT8   *SrcBuffer,
  IN      UINT32_EFI  SrcSize,
  IN      UINT8   *DstBuffer,
  IN OUT  UINT32_EFI  *DstSize
  );

//
// Decompression algorithm begins here
//
#define BITBUFSIZ 32
#define MAXMATCH  256
#define THRESHOLD 3
#define CODE_BIT  16
#define BAD_TABLE - 1

//
// C: Char&Len Set; P: Position Set; T: exTra Set
//
#define NC      (0xff + MAXMATCH + 2 - THRESHOLD)
#define CBIT    9
#define MAXPBIT 5
#define TBIT    5
#define MAXNP   ((1U << MAXPBIT) - 1)
#define NT      (CODE_BIT + 3)
#if NT > MAXNP
#define NPT NT
#else
#define NPT MAXNP
#endif

typedef struct {
  UINT8   *mSrcBase;  // Starting address of compressed data
  UINT8   *mDstBase;  // Starting address of decompressed data
  std::istream* mIstrm;  // input stream for compressed data
  std::ostream* mOstrm;  // output stream for decompressed data
  UINT8*  mOstrmWin;  // window into output stream
  UINT32_EFI mOstrmWinStartidx;
  UINT32_EFI  mOutBuf;
  UINT32_EFI  mInBuf;

  UINT16  mBitCount;
  UINT32_EFI  mBitBuf;
  UINT32_EFI  mSubBitBuf;
  UINT16  mBlockSize;
  UINT32_EFI  mCompSize;
  UINT32_EFI  mOrigSize;

  UINT16  mBadTableFlag;

  UINT16  mLeft[2 * NC - 1];
  UINT16  mRight[2 * NC - 1];
  UINT8   mCLen[NC];
  UINT8   mPTLen[NPT];
  UINT16  mCTable[4096];
  UINT16  mPTTable[256];

  //
  // The length of the field 'Position Set Code Length Array Size' in Block Header.
  // For EFI 1.1 de/compression algorithm, mPBit = 4
  // For Tiano de/compression algorithm, mPBit = 5
  //
  UINT8   mPBit;
} SCRATCH_DATA;

EFI_STATUS
EFIAPI
EfiGetInfo (
  IN      VOID                    *Source,
  IN      UINT32_EFI                  SrcSize,
  OUT     UINT32_EFI                  *DstSize,
  OUT     UINT32_EFI                  *ScratchSize
  )
/*++

Routine Description:

  The implementation is same as that  of EFI_DECOMPRESS_PROTOCOL.GetInfo().

Arguments:

  This        - The protocol instance pointer
  Source      - The source buffer containing the compressed data.
  SrcSize     - The size of source buffer
  DstSize     - The size of destination buffer.
  ScratchSize - The size of scratch buffer.

Returns:

  EFI_SUCCESS           - The size of destination buffer and the size of scratch buffer are successull retrieved.
  EFI_INVALID_PARAMETER - The source data is corrupted

--*/
;

EFI_STATUS
EFIAPI
EfiGetInfo (
  IN      std::istream&               istrm,
  OUT     UINT32_EFI                  *DstSize,
  OUT     UINT32_EFI                  *ScratchSize
  )
/*++

Routine Description:

  The implementation is same as that  of EFI_DECOMPRESS_PROTOCOL.GetInfo().

Arguments:

  This        - The protocol instance pointer
  istrm       - The source stream containing the compressed data.
  DstSize     - The size of destination buffer.
  ScratchSize - The size of scratch buffer.

Returns:

  EFI_SUCCESS           - The size of destination buffer and the size of scratch buffer are successull retrieved.
  EFI_INVALID_PARAMETER - The source data is corrupted

--*/
;

EFI_STATUS
EFIAPI
EfiDecompress (
  IN      VOID                    *Source,
  IN      UINT32_EFI                  SrcSize,
  IN OUT  VOID                    *Destination,
  IN      UINT32_EFI                  DstSize,
  IN OUT  VOID                    *Scratch,
  IN      UINT32_EFI                  ScratchSize
  )
/*++

Routine Description:

  The implementation is same as that  of EFI_TIANO_DECOMPRESS_PROTOCOL.Decompress().

Arguments:

  This        - The protocol instance pointer
  Source      - The source buffer containing the compressed data.
  SrcSize     - The size of source buffer
  Destination - The destination buffer to store the decompressed data
  DstSize     - The size of destination buffer.
  Scratch     - The buffer used internally by the decompress routine. This  buffer is needed to store intermediate data.
  ScratchSize - The size of scratch buffer.

Returns:

  EFI_SUCCESS           - Decompression is successfull
  EFI_INVALID_PARAMETER - The source data is corrupted

--*/
;

EFI_STATUS
EFIAPI
EfiDecompress (
  IN      VOID                    *Source,
  IN      UINT32_EFI               SrcSize,
  IN OUT  std::ostream&            ostrm,
  IN OUT  VOID                    *Scratch,
  IN      UINT32_EFI                  ScratchSize
  )
/*++

Routine Description:

  The implementation is same as that  of EFI_TIANO_DECOMPRESS_PROTOCOL.Decompress().

Arguments:

  This        - The protocol instance pointer
  Source      - The source buffer containing the compressed data.
  SrcSize     - The size of source buffer
  ostrm       - The output stream to contain the decompressed data
  Scratch     - The buffer used internally by the decompress routine. This  buffer is needed to store intermediate data.
  ScratchSize - The size of scratch buffer.

Returns:

  EFI_SUCCESS           - Decompression is successfull
  EFI_INVALID_PARAMETER - The source data is corrupted

--*/
;

EFI_STATUS
EFIAPI
EfiDecompress (
  IN      std::istream&            istrm,
  IN OUT  std::ostream&            ostrm,
  IN OUT  VOID                    *Scratch,
  IN      UINT32_EFI               ScratchSize
  )
/*++

Routine Description:

  The implementation is same as that  of EFI_TIANO_DECOMPRESS_PROTOCOL.Decompress().

Arguments:

  This        - The protocol instance pointer
  istrm       - The input stream containing the compressed data
  ostrm       - The output stream to contain the decompressed data
  Scratch     - The buffer used internally by the decompress routine. This  buffer is needed to store intermediate data.
  ScratchSize - The size of scratch buffer.

Returns:

  EFI_SUCCESS           - Decompression is successfull
  EFI_INVALID_PARAMETER - The source data is corrupted

--*/
;

typedef
EFI_STATUS
(*GETINFO_FUNCTION) (
  IN      VOID    *Source,
  IN      UINT32_EFI  SrcSize,
  OUT     UINT32_EFI  *DstSize,
  OUT     UINT32_EFI  *ScratchSize
  );

typedef
EFI_STATUS
(*DECOMPRESS_FUNCTION) (
  IN      VOID    *Source,
  IN      UINT32_EFI  SrcSize,
  IN OUT  VOID    *Destination,
  IN      UINT32_EFI  DstSize,
  IN OUT  VOID    *Scratch,
  IN      UINT32_EFI  ScratchSize
  );


#endif
