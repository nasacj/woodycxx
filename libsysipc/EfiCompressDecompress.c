
/*

Copyright (c) 2006, Intel Corporation                                              
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  EfiCompress.c

Abstract:

  Compression routine. The compression algorithm is a mixture of
  LZ77 and Huffman coding. LZ77 transforms the source data into a
  sequence of Original Characters and Pointers to repeated strings.
  This sequence is further divided into Blocks and Huffman codings
  are applied to each Block.

--*/

#define __STDC_LIMIT_MACROS   // include stdint macros

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EfiCompressDecompress.h"

//
// Macro Definitions
//

typedef INT16             NODE;
#define UINT8_BIT         8
#define THRESHOLD         3
#define INIT_CRC          0
#define WNDBIT            13
#define WNDSIZ            (1 << WNDBIT)
#define MAXMATCH          256
#define PERC_FLAG         0x8000U
#define CODE_BIT          16
#define NIL               0
#define MAX_HASH_VAL      (3 * WNDSIZ + (WNDSIZ / 512 + 1) * UINT8_MAX)
#define HASH(p, c)        ((p) + ((c) << (WNDBIT - 9)) + WNDSIZ * 2)
#define CRCPOLY           0xA001
#define UPDATE_CRC(c)     mCrc = mCrcTable[(mCrc ^ (c)) & 0xFF] ^ (mCrc >> UINT8_BIT)

//
// C: the Char&Len Set; P: the Position Set; T: the exTra Set
//

#define CBIT              9
#define NP                (WNDBIT + 1)
//#define PBIT              4
#define PBIT              5
#define NT                (CODE_BIT + 3)
#define TBIT              5

//#if NT > NP
//  #define                 NPT NT
//#else
//  #define                 NPT NP
//#endif

//
// Function Prototypes
//

STATIC
VOID 
PutChar(
  IN UINT8 Char
  );

STATIC
VOID 
PutDword(
  IN UINT32_EFI Data,
  IN UINT32_EFI Offset
  );

STATIC
EFI_STATUS 
AllocateMemory (
  );

STATIC
VOID
FreeMemory (
  );

STATIC 
VOID 
InitSlide (
  );

STATIC 
NODE 
Child (
  IN NODE q, 
  IN UINT8 c
  );

STATIC 
VOID 
MakeChild (
  IN NODE q, 
  IN UINT8 c, 
  IN NODE r
  );
  
STATIC 
VOID 
Split (
  IN NODE Old
  );

STATIC 
VOID 
InsertNode (
  );
  
STATIC 
VOID 
DeleteNode (
  );

STATIC 
VOID 
GetNextMatch (
  );
  
STATIC 
EFI_STATUS 
Encode (
  );

STATIC 
VOID 
CountTFreq (
  );

STATIC 
VOID 
WritePTLen (
  IN INT32_EFI n, 
  IN INT32_EFI nbit, 
  IN INT32_EFI Special
  );

STATIC 
VOID 
WriteCLen (
  );
  
STATIC 
VOID 
EncodeC (
  IN INT32_EFI c
  );

STATIC 
VOID 
EncodeP (
  IN UINT32_EFI p
  );

STATIC 
VOID 
SendBlock (
  );
  
STATIC 
VOID 
Output (
  IN UINT32_EFI c, 
  IN UINT32_EFI p
  );

STATIC 
VOID 
HufEncodeStart (
  );
  
STATIC 
VOID 
HufEncodeEnd (
  );
  
STATIC 
VOID 
MakeCrcTable (
  );
  
STATIC 
VOID 
PutBits (
  IN INT32_EFI n, 
  IN UINT32_EFI x
  );
  
STATIC 
INT32_EFI 
FreadCrc (
  OUT UINT8 *p, 
  IN  INT32_EFI n
  );
  
STATIC 
VOID 
InitPutBits (
  );
  
STATIC 
VOID 
CountLen (
  IN INT32_EFI i
  );

STATIC 
VOID 
MakeLen (
  IN INT32_EFI Root
  );
  
STATIC 
VOID 
DownHeap (
  IN INT32_EFI i
  );

STATIC 
VOID 
MakeCode (
  IN  INT32_EFI n, 
  IN  UINT8 Len[], 
  OUT UINT16 Code[]
  );
  
STATIC 
INT32_EFI 
MakeTree (
  IN  INT32_EFI   NParm, 
  IN  UINT16  FreqParm[], 
  OUT UINT8   LenParm[], 
  OUT UINT16  CodeParm[]
  );


//
//  Global Variables
//

STATIC UINT8  *mSrc, *mDst, *mSrcUpperLimit, *mDstUpperLimit;
STATIC std::istream* mIstrm;
STATIC std::ostream* mOstrm;

STATIC UINT8  *mLevel, *mText, *mChildCount, *mBuf, mCLen[NC], mPTLen[NPT], *mLen;
STATIC INT16  mHeap[NC + 1];
STATIC INT32_EFI  mRemainder, mMatchLen, mBitCount, mHeapSize, mN;
STATIC UINT32_EFI mBufSiz = 0, mOutputPos, mOutputMask, mSubBitBuf, mCrc;
STATIC UINT32_EFI mCompSize, mOrigSize;

STATIC UINT16 *mFreq, *mSortPtr, mLenCnt[17], mLeft[2 * NC - 1], mRight[2 * NC - 1],
              mCrcTable[UINT8_MAX + 1], mCFreq[2 * NC - 1], /*mCTable[4096],*/ mCCode[NC],
              mPFreq[2 * NP - 1], mPTCode[NPT], mTFreq[2 * NT - 1];

STATIC NODE   mPos, mMatchPos, mAvail, *mPosition, *mParent, *mPrev, *mNext = NULL;


//
// functions
//

EFI_STATUS
_EfiCompress (
  IN      UINT8   *SrcBuffer,
  IN      UINT32_EFI  SrcSize,
  IN      UINT8   *DstBuffer,
  IN OUT  UINT32_EFI  *DstSize,
  IN      std::istream*  istrm,
  IN      std::ostream*  ostrm
  )
/*++

Routine Description:

  The main compression routine.

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

--*/
{
  EFI_STATUS Status = EFI_SUCCESS;
  
  //
  // Initializations
  //
  mBufSiz = 0;
  mBuf = NULL;
  mText       = NULL;
  mLevel      = NULL;
  mChildCount = NULL;
  mPosition   = NULL;
  mParent     = NULL;
  mPrev       = NULL;
  mNext       = NULL;

  
  mSrc = SrcBuffer;
  mSrcUpperLimit = mSrc + SrcSize;
  mDst = DstBuffer;
  if (DstSize) 
    mDstUpperLimit = mDst + *DstSize;
  else 
    mDstUpperLimit = 0;

  mIstrm = istrm;
  mOstrm = ostrm;

  PutDword(0L, 0);
  PutDword(0L, 4);
  
  MakeCrcTable ();

  mOrigSize = mCompSize = 0;
  mCrc = INIT_CRC;
  
  //
  // Compress it
  //
  
  Status = Encode();
  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  //
  // Null terminate the compressed data
  //
  PutChar(0);
  
  //
  // Fill in compressed size and original size
  //
  mDst = DstBuffer;
  PutDword(mCompSize+1, 0);
  PutDword(mOrigSize, 4);

  //
  // Return
  //
  
  if (DstSize) {
    if (mCompSize + 1 + 8 > *DstSize) {
      *DstSize = mCompSize + 1 + 8;
      return EFI_BUFFER_TOO_SMALL;
    } else {
      *DstSize = mCompSize + 1 + 8;
      return EFI_SUCCESS;
    }
  }

  return EFI_SUCCESS;

}


EFI_STATUS
EfiCompress (
  IN      UINT8   *SrcBuffer,
  IN      UINT32_EFI  SrcSize,
  IN      UINT8   *DstBuffer,
  IN OUT  UINT32_EFI  *DstSize
  )
{
  return _EfiCompress (
  SrcBuffer,
  SrcSize,
  DstBuffer,
  DstSize,
  0,
  0);
}

EFI_STATUS
EfiCompress (
  IN      UINT8   *SrcBuffer,
  IN      UINT32_EFI  SrcSize,
  IN      std::ostream&  mOstrm
  )
{
  return _EfiCompress (
  SrcBuffer,
  SrcSize,
  0,
  0,
  0,
  &mOstrm);
}

EFI_STATUS
EfiCompress (
  IN      std::istream&  mIstrm,
  IN      std::ostream&  mOstrm
  )
{
  return _EfiCompress (
  0,
  0,
  0,
  0,
  &mIstrm,
  &mOstrm);
}


STATIC
VOID 
PutChar(
  IN UINT8 Char
  )
{
  //fprintf(stderr, "putchar %02x\n", (unsigned int) Char);
  if (mDst && mDst < mDstUpperLimit) {
      *mDst++ = Char;
  } else if (mOstrm) {
    mOstrm->put( Char );
  }
}

STATIC 
VOID 
PutDword(
  IN UINT32_EFI Data,
  IN UINT32_EFI Offset
  )
/*++

Routine Description:

  Put a dword to output stream
  
Arguments:

  Offset  - the offset to put data
  Data    - the dword to put
  
Returns: (VOID)
  
--*/
{
  if (mDst) {
    if (mDst < mDstUpperLimit) {
      *mDst++ = (UINT8)(((UINT8)(Data        )) & 0xff);
    }

    if (mDst < mDstUpperLimit) {
      *mDst++ = (UINT8)(((UINT8)(Data >> 0x08)) & 0xff);
    }

    if (mDst < mDstUpperLimit) {
      *mDst++ = (UINT8)(((UINT8)(Data >> 0x10)) & 0xff);
    }

    if (mDst < mDstUpperLimit) {
      *mDst++ = (UINT8)(((UINT8)(Data >> 0x18)) & 0xff);
    }
  } 
  else if (mOstrm) {

    char dword[4];
    dword[0] = (UINT8)(((UINT8)(Data        )) & 0xff);
    dword[1] = (UINT8)(((UINT8)(Data >> 0x08)) & 0xff);
    dword[2] = (UINT8)(((UINT8)(Data >> 0x10)) & 0xff);
    dword[3] = (UINT8)(((UINT8)(Data >> 0x18)) & 0xff);

    mOstrm->seekp(Offset);
    mOstrm->write ( dword,4 );
  }
}


STATIC 
UINT32_EFI 
GetDword(
   UINT8* Src,
   IN UINT32_EFI Offset
  )
/*++

Routine Description:

  Get a dword from memory
  
Arguments:

  Src     - prt to memory
  Offset  - the offset of data
  
Returns: dword at offset
  
--*/
{
  UINT32_EFI Val = 0;
  Src += Offset;
  Val = Src[0] + (Src[1] << 8) + (Src[2] << 16) + (Src[3] << 24);
  return Val;
}

STATIC 
UINT32_EFI 
GetDword(
   std::istream* istrm,
   IN UINT32_EFI Offset
  )
/*++

Routine Description:

  Get a dword from input stream
  
Arguments:

  istrm   - prt to input stream
  Offset  - the offset of data
  
Returns: dword at offset
  
--*/
{
  UINT32_EFI Val = 0;
  char dw[5] = {0};
  UINT8* Src = (UINT8*) dw;
  istrm->seekg(Offset);
  istrm->read(dw,4);
  Val = Src[0] + (Src[1] << 8) + (Src[2] << 16) + (Src[3] << 24);
  return Val;
}


STATIC
EFI_STATUS
AllocateMemory ()
/*++

Routine Description:

  Allocate memory spaces for data structures used in compression process
  
Argements: (VOID)

Returns:

  EFI_SUCCESS           - Memory is allocated successfully
  EFI_OUT_OF_RESOURCES  - Allocation fails

--*/
{
  UINT32_EFI      i;
  
  mText       = (UINT8*) malloc (WNDSIZ * 2 + MAXMATCH);
  for (i = 0 ; i < WNDSIZ * 2 + MAXMATCH; i ++) {
    mText[i] = 0;
  }

  mLevel      = (UINT8*)malloc ((WNDSIZ + UINT8_MAX + 1) * sizeof(*mLevel));
  mChildCount = (UINT8*)malloc ((WNDSIZ + UINT8_MAX + 1) * sizeof(*mChildCount));
  mPosition   = (NODE*)malloc ((WNDSIZ + UINT8_MAX + 1) * sizeof(*mPosition));
  mParent     = (NODE*)malloc (WNDSIZ * 2 * sizeof(*mParent));
  mPrev       = (NODE*)malloc (WNDSIZ * 2 * sizeof(*mPrev));
  mNext       = (NODE*)malloc ((MAX_HASH_VAL + 1) * sizeof(*mNext));
  
  mBufSiz = 16 * 1024U;
  while ((mBuf = (UINT8*)malloc(mBufSiz)) == NULL) {
    mBufSiz = (mBufSiz / 10U) * 9U;
    if (mBufSiz < 4 * 1024U) {
      return EFI_OUT_OF_RESOURCES;
    }
  }
  mBuf[0] = 0;
  
  return EFI_SUCCESS;
}

VOID
FreeMemory ()
/*++

Routine Description:

  Called when compression is completed to free memory previously allocated.
  
Arguments: (VOID)

Returns: (VOID)

--*/
{
  if (mText) {
    free (mText);
  }
  
  if (mLevel) {
    free (mLevel);
  }
  
  if (mChildCount) {
    free (mChildCount);
  }
  
  if (mPosition) {
    free (mPosition);
  }
  
  if (mParent) {
    free (mParent);
  }
  
  if (mPrev) {
    free (mPrev);
  }
  
  if (mNext) {
    free (mNext);
  }
  
  if (mBuf) {
    free (mBuf);
  }  

  return;
}


STATIC 
VOID 
InitSlide ()
/*++

Routine Description:

  Initialize String Info Log data structures
  
Arguments: (VOID)

Returns: (VOID)

--*/
{
  NODE i;

  for (i = WNDSIZ; i <= WNDSIZ + UINT8_MAX; i++) {
    mLevel[i] = 1;
    mPosition[i] = NIL;  /* sentinel */
  }
  for (i = WNDSIZ; i < WNDSIZ * 2; i++) {
    mParent[i] = NIL;
  }  
  mAvail = 1;
  for (i = 1; i < WNDSIZ - 1; i++) {
    mNext[i] = (NODE)(i + 1);
  }
  
  mNext[WNDSIZ - 1] = NIL;
  for (i = WNDSIZ * 2; i <= MAX_HASH_VAL; i++) {
    mNext[i] = NIL;
  }  
}


STATIC 
NODE 
Child (
  IN NODE q, 
  IN UINT8 c
  )
/*++

Routine Description:

  Find child node given the parent node and the edge character
  
Arguments:

  q       - the parent node
  c       - the edge character
  
Returns:

  The child node (NIL if not found)  
  
--*/
{
  NODE r;
  
  r = mNext[HASH(q, c)];
  mParent[NIL] = q;  /* sentinel */
  while (mParent[r] != q) {
    r = mNext[r];
  }
  
  return r;
}

STATIC 
VOID 
MakeChild (
  IN NODE q, 
  IN UINT8 c, 
  IN NODE r
  )
/*++

Routine Description:

  Create a new child for a given parent node.
  
Arguments:

  q       - the parent node
  c       - the edge character
  r       - the child node
  
Returns: (VOID)

--*/
{
  NODE h, t;
  
  h = (NODE)HASH(q, c);
  t = mNext[h];
  mNext[h] = r;
  mNext[r] = t;
  mPrev[t] = r;
  mPrev[r] = h;
  mParent[r] = q;
  mChildCount[q]++;
}

STATIC 
VOID 
Split (
  NODE Old
  )
/*++

Routine Description:

  Split a node.
  
Arguments:

  Old     - the node to split
  
Returns: (VOID)

--*/
{
  NODE New, t;

  New = mAvail;
  mAvail = mNext[New];
  mChildCount[New] = 0;
  t = mPrev[Old];
  mPrev[New] = t;
  mNext[t] = New;
  t = mNext[Old];
  mNext[New] = t;
  mPrev[t] = New;
  mParent[New] = mParent[Old];
  mLevel[New] = (UINT8)mMatchLen;
  mPosition[New] = mPos;
  MakeChild(New, mText[mMatchPos + mMatchLen], Old);
  MakeChild(New, mText[mPos + mMatchLen], mPos);
}

STATIC 
VOID 
InsertNode ()
/*++

Routine Description:

  Insert string info for current position into the String Info Log
  
Arguments: (VOID)

Returns: (VOID)

--*/
{
  NODE q, r, j, t;
  UINT8 c, *t1, *t2;

  if (mMatchLen >= 4) {
    
    //
    // We have just got a long match, the target tree
    // can be located by MatchPos + 1. Travese the tree
    // from bottom up to get to a proper starting point.
    // The usage of PERC_FLAG ensures proper node deletion
    // in DeleteNode() later.
    //
    
    mMatchLen--;
    r = (INT16)((mMatchPos + 1) | WNDSIZ);
    while ((q = mParent[r]) == NIL) {
      r = mNext[r];
    }
    while (mLevel[q] >= mMatchLen) {
      r = q;  q = mParent[q];
    }
    t = q;
    while (mPosition[t] < 0) {
      mPosition[t] = mPos;
      t = mParent[t];
    }
    if (t < WNDSIZ) {
      mPosition[t] = (NODE)(mPos | PERC_FLAG);
    }    
  } else {
    
    //
    // Locate the target tree
    //
    
    q = (INT16)(mText[mPos] + WNDSIZ);
    c = mText[mPos + 1];
    if ((r = Child(q, c)) == NIL) {
      MakeChild(q, c, mPos);
      mMatchLen = 1;
      return;
    }
    mMatchLen = 2;
  }
  
  //
  // Traverse down the tree to find a match.
  // Update Position value along the route.
  // Node split or creation is involved.
  //
  
  for ( ; ; ) {
    if (r >= WNDSIZ) {
      j = MAXMATCH;
      mMatchPos = r;
    } else {
      j = mLevel[r];
      mMatchPos = (NODE)(mPosition[r] & ~PERC_FLAG);
    }
    if (mMatchPos >= mPos) {
      mMatchPos -= WNDSIZ;
    }    
    t1 = &mText[mPos + mMatchLen];
    t2 = &mText[mMatchPos + mMatchLen];
    while (mMatchLen < j) {
      if (*t1 != *t2) {
        Split(r);
        return;
      }
      mMatchLen++;
      t1++;
      t2++;
    }
    if (mMatchLen >= MAXMATCH) {
      break;
    }
    mPosition[r] = mPos;
    q = r;
    if ((r = Child(q, *t1)) == NIL) {
      MakeChild(q, *t1, mPos);
      return;
    }
    mMatchLen++;
  }
  t = mPrev[r];
  mPrev[mPos] = t;
  mNext[t] = mPos;
  t = mNext[r];
  mNext[mPos] = t;
  mPrev[t] = mPos;
  mParent[mPos] = q;
  mParent[r] = NIL;
  
  //
  // Special usage of 'next'
  //
  mNext[r] = mPos;
  
}

STATIC 
VOID 
DeleteNode ()
/*++

Routine Description:

  Delete outdated string info. (The Usage of PERC_FLAG
  ensures a clean deletion)
  
Arguments: (VOID)

Returns: (VOID)

--*/
{
  NODE q, r, s, t, u;

  if (mParent[mPos] == NIL) {
    return;
  }
  
  r = mPrev[mPos];
  s = mNext[mPos];
  mNext[r] = s;
  mPrev[s] = r;
  r = mParent[mPos];
  mParent[mPos] = NIL;
  if (r >= WNDSIZ || --mChildCount[r] > 1) {
    return;
  }
  t = (NODE)(mPosition[r] & ~PERC_FLAG);
  if (t >= mPos) {
    t -= WNDSIZ;
  }
  s = t;
  q = mParent[r];
  while ((u = mPosition[q]) & PERC_FLAG) {
    u &= ~PERC_FLAG;
    if (u >= mPos) {
      u -= WNDSIZ;
    }
    if (u > s) {
      s = u;
    }
    mPosition[q] = (INT16)(s | WNDSIZ);
    q = mParent[q];
  }
  if (q < WNDSIZ) {
    if (u >= mPos) {
      u -= WNDSIZ;
    }
    if (u > s) {
      s = u;
    }
    mPosition[q] = (INT16)(s | WNDSIZ | PERC_FLAG);
  }
  s = Child(r, mText[t + mLevel[r]]);
  t = mPrev[s];
  u = mNext[s];
  mNext[t] = u;
  mPrev[u] = t;
  t = mPrev[r];
  mNext[t] = s;
  mPrev[s] = t;
  t = mNext[r];
  mPrev[t] = s;
  mNext[s] = t;
  mParent[s] = mParent[r];
  mParent[r] = NIL;
  mNext[r] = mAvail;
  mAvail = r;
}

STATIC 
VOID 
GetNextMatch ()
/*++

Routine Description:

  Advance the current position (read in new data if needed).
  Delete outdated string info. Find a match string for current position.

Arguments: (VOID)

Returns: (VOID)

--*/
{
  INT32_EFI n;

  mRemainder--;
  if (++mPos == WNDSIZ * 2) {
    memmove(&mText[0], &mText[WNDSIZ], WNDSIZ + MAXMATCH);
    n = FreadCrc(&mText[WNDSIZ + MAXMATCH], WNDSIZ);
    mRemainder += n;
    mPos = WNDSIZ;
  }
  DeleteNode();
  InsertNode();
}

STATIC
EFI_STATUS
Encode ()
/*++

Routine Description:

  The main controlling routine for compression process.

Arguments: (VOID)

Returns:
  
  EFI_SUCCESS           - The compression is successful
  EFI_OUT_0F_RESOURCES  - Not enough memory for compression process

--*/
{
  EFI_STATUS  Status;
  INT32_EFI       LastMatchLen;
  NODE        LastMatchPos;

  Status = AllocateMemory();
  if (EFI_ERROR(Status)) {
    FreeMemory();
    return Status;
  }

  InitSlide();
  
  HufEncodeStart();

  mRemainder = FreadCrc(&mText[WNDSIZ], WNDSIZ + MAXMATCH);
  
  mMatchLen = 0;
  mPos = WNDSIZ;
  InsertNode();
  if (mMatchLen > mRemainder) {
    mMatchLen = mRemainder;
  }
  while (mRemainder > 0) {
    LastMatchLen = mMatchLen;
    LastMatchPos = mMatchPos;
    GetNextMatch();
    if (mMatchLen > mRemainder) {
      mMatchLen = mRemainder;
    }
    
    if (mMatchLen > LastMatchLen || LastMatchLen < THRESHOLD) {
      
      //
      // Not enough benefits are gained by outputting a pointer,
      // so just output the original character
      //
      
      Output(mText[mPos - 1], 0);
    } else {
      
      //
      // Outputting a pointer is beneficial enough, do it.
      //
      
      Output(LastMatchLen + (UINT8_MAX + 1 - THRESHOLD),
             (mPos - LastMatchPos - 2) & (WNDSIZ - 1));
      while (--LastMatchLen > 0) {
        GetNextMatch();
      }
      if (mMatchLen > mRemainder) {
        mMatchLen = mRemainder;
      }
    }
  }
  
  HufEncodeEnd();
  FreeMemory();
  return EFI_SUCCESS;
}

STATIC 
VOID 
CountTFreq ()
/*++

Routine Description:

  Count the frequencies for the Extra Set
  
Arguments: (VOID)

Returns: (VOID)

--*/
{
  INT32_EFI i, k, n, Count;

  for (i = 0; i < NT; i++) {
    mTFreq[i] = 0;
  }
  n = NC;
  while (n > 0 && mCLen[n - 1] == 0) {
    n--;
  }
  i = 0;
  while (i < n) {
    k = mCLen[i++];
    if (k == 0) {
      Count = 1;
      while (i < n && mCLen[i] == 0) {
        i++;
        Count++;
      }
      if (Count <= 2) {
        mTFreq[0] = (UINT16)(mTFreq[0] + Count);
      } else if (Count <= 18) {
        mTFreq[1]++;
      } else if (Count == 19) {
        mTFreq[0]++;
        mTFreq[1]++;
      } else {
        mTFreq[2]++;
      }
    } else {
      mTFreq[k + 2]++;
    }
  }
}

STATIC 
VOID 
WritePTLen (
  IN INT32_EFI n, 
  IN INT32_EFI nbit, 
  IN INT32_EFI Special
  )
/*++

Routine Description:

  Outputs the code length array for the Extra Set or the Position Set.
  
Arguments:

  n       - the number of symbols
  nbit    - the number of bits needed to represent 'n'
  Special - the special symbol that needs to be take care of
  
Returns: (VOID)

--*/
{
  INT32_EFI i, k;

  while (n > 0 && mPTLen[n - 1] == 0) {
    n--;
  }
  PutBits(nbit, n);
  i = 0;
  while (i < n) {
    k = mPTLen[i++];
    if (k <= 6) {
      PutBits(3, k);
    } else {
      PutBits(k - 3, (1U << (k - 3)) - 2);
    }
    if (i == Special) {
      while (i < 6 && mPTLen[i] == 0) {
        i++;
      }
      PutBits(2, (i - 3) & 3);
    }
  }
}

STATIC 
VOID 
WriteCLen ()
/*++

Routine Description:

  Outputs the code length array for Char&Length Set
  
Arguments: (VOID)

Returns: (VOID)

--*/
{
  INT32_EFI i, k, n, Count;

  n = NC;
  while (n > 0 && mCLen[n - 1] == 0) {
    n--;
  }
  PutBits(CBIT, n);
  i = 0;
  while (i < n) {
    k = mCLen[i++];
    if (k == 0) {
      Count = 1;
      while (i < n && mCLen[i] == 0) {
        i++;
        Count++;
      }
      if (Count <= 2) {
        for (k = 0; k < Count; k++) {
          PutBits(mPTLen[0], mPTCode[0]);
        }
      } else if (Count <= 18) {
        PutBits(mPTLen[1], mPTCode[1]);
        PutBits(4, Count - 3);
      } else if (Count == 19) {
        PutBits(mPTLen[0], mPTCode[0]);
        PutBits(mPTLen[1], mPTCode[1]);
        PutBits(4, 15);
      } else {
        PutBits(mPTLen[2], mPTCode[2]);
        PutBits(CBIT, Count - 20);
      }
    } else {
      PutBits(mPTLen[k + 2], mPTCode[k + 2]);
    }
  }
}

STATIC 
VOID 
EncodeC (
  IN INT32_EFI c
  )
{
  PutBits(mCLen[c], mCCode[c]);
}

STATIC 
VOID 
EncodeP (
  IN UINT32_EFI p
  )
{
  UINT32_EFI c, q;

  c = 0;
  q = p;
  while (q) {
    q >>= 1;
    c++;
  }
  PutBits(mPTLen[c], mPTCode[c]);
  if (c > 1) {
    PutBits(c - 1, p & (0xFFFFU >> (17 - c)));
  }
}

STATIC 
VOID 
SendBlock ()
/*++

Routine Description:

  Huffman code the block and output it.
  
Argument: (VOID)

Returns: (VOID)

--*/
{
  UINT32_EFI i, k, Flags, Root, Pos, Size;
  Flags = 0;

  Root = MakeTree(NC, mCFreq, mCLen, mCCode);
  Size = mCFreq[Root];
  PutBits(16, Size);
  if (Root >= NC) {
    CountTFreq();
    Root = MakeTree(NT, mTFreq, mPTLen, mPTCode);
    if (Root >= NT) {
      WritePTLen(NT, TBIT, 3);
    } else {
      PutBits(TBIT, 0);
      PutBits(TBIT, Root);
    }
    WriteCLen();
  } else {
    PutBits(TBIT, 0);
    PutBits(TBIT, 0);
    PutBits(CBIT, 0);
    PutBits(CBIT, Root);
  }
  Root = MakeTree(NP, mPFreq, mPTLen, mPTCode);
  if (Root >= NP) {
    WritePTLen(NP, PBIT, -1);
  } else {
    PutBits(PBIT, 0);
    PutBits(PBIT, Root);
  }
  Pos = 0;
  for (i = 0; i < Size; i++) {
    if (i % UINT8_BIT == 0) {
      Flags = mBuf[Pos++];
    } else {
      Flags <<= 1;
    }
    if (Flags & (1U << (UINT8_BIT - 1))) {
      EncodeC(mBuf[Pos++] + (1U << UINT8_BIT));
      k = mBuf[Pos++] << UINT8_BIT;
      k += mBuf[Pos++];
      EncodeP(k);
    } else {
      EncodeC(mBuf[Pos++]);
    }
  }
  for (i = 0; i < NC; i++) {
    mCFreq[i] = 0;
  }
  for (i = 0; i < NP; i++) {
    mPFreq[i] = 0;
  }
}


STATIC 
VOID 
Output (
  IN UINT32_EFI c, 
  IN UINT32_EFI p
  )
/*++

Routine Description:

  Outputs an Original Character or a Pointer

Arguments:

  c     - The original character or the 'String Length' element of a Pointer
  p     - The 'Position' field of a Pointer

Returns: (VOID)

--*/
{
  STATIC UINT32_EFI CPos;

  if ((mOutputMask >>= 1) == 0) {
    mOutputMask = 1U << (UINT8_BIT - 1);
    if (mOutputPos >= mBufSiz - 3 * UINT8_BIT) {
      SendBlock();
      mOutputPos = 0;
    }
    CPos = mOutputPos++;  
    mBuf[CPos] = 0;
  }
  mBuf[mOutputPos++] = (UINT8) c;
  mCFreq[c]++;
  if (c >= (1U << UINT8_BIT)) {
    mBuf[CPos] |= mOutputMask;
    mBuf[mOutputPos++] = (UINT8)(p >> UINT8_BIT);
    mBuf[mOutputPos++] = (UINT8) p;
    c = 0;
    while (p) {
      p >>= 1;
      c++;
    }
    mPFreq[c]++;
  }
}

STATIC
VOID
HufEncodeStart ()
{
  INT32_EFI i;

  for (i = 0; i < NC; i++) {
    mCFreq[i] = 0;
  }
  for (i = 0; i < NP; i++) {
    mPFreq[i] = 0;
  }
  mOutputPos = mOutputMask = 0;
  InitPutBits();
  return;
}

STATIC 
VOID 
HufEncodeEnd ()
{
  SendBlock();
  
  //
  // Flush remaining bits
  //
  PutBits(UINT8_BIT - 1, 0);
  
  return;
}


STATIC 
VOID 
MakeCrcTable ()
{
  UINT32_EFI i, j, r;

  for (i = 0; i <= UINT8_MAX; i++) {
    r = i;
    for (j = 0; j < UINT8_BIT; j++) {
      if (r & 1) {
        r = (r >> 1) ^ CRCPOLY;
      } else {
        r >>= 1;
      }
    }
    mCrcTable[i] = (UINT16)r;    
  }
}

STATIC 
VOID 
PutBits (
  IN INT32_EFI n, 
  IN UINT32_EFI x
  )
/*++

Routine Description:

  Outputs rightmost n bits of x

Argments:

  n   - the rightmost n bits of the data is used
  x   - the data 

Returns: (VOID)

--*/
{
  UINT8 Temp;  
  
  if (n < mBitCount) {
    mSubBitBuf |= x << (mBitCount -= n);
  } else {
      
    Temp = (UINT8)(mSubBitBuf | (x >> (n -= mBitCount)));
    PutChar(Temp);
    mCompSize++;

    if (n < UINT8_BIT) {
      mSubBitBuf = x << (mBitCount = UINT8_BIT - n);
    } else {
        
      Temp = (UINT8)(x >> (n - UINT8_BIT));
      PutChar(Temp);
      mCompSize++;
      
      mSubBitBuf = x << (mBitCount = 2 * UINT8_BIT - n);
    }
  }
}

STATIC 
INT32_EFI 
FreadCrc (
  OUT UINT8 *p, 
  IN  INT32_EFI n
  )
/*++

Routine Description:

  Read in source data
  
Arguments:

  p   - the buffer to hold the data
  n   - number of bytes to read

Returns:

  number of bytes actually read
  
--*/
{
  INT32_EFI i = 0;

  if (mSrc) {
    for (i = 0; mSrc < mSrcUpperLimit && i < n; i++) {
      *p++ = *mSrc++;
    }
  } else if (mIstrm) {
    for (i = 0; mIstrm->peek() != EOF && i < n; i++) {
      *p++ = mIstrm->get();
    }
  }
  //fprintf(stderr, "freadcrc read n=%d\n", (unsigned int) i);
  n = i;

  p -= n;
  mOrigSize += n;
  while (--i >= 0) {
    UPDATE_CRC(*p++);
  }
  return n;
}


STATIC 
VOID 
InitPutBits ()
{
  mBitCount = UINT8_BIT;  
  mSubBitBuf = 0;
}

STATIC 
VOID 
CountLen (
  IN INT32_EFI i
  )
/*++

Routine Description:

  Count the number of each code length for a Huffman tree.
  
Arguments:

  i   - the top node
  
Returns: (VOID)

--*/
{
  STATIC INT32_EFI Depth = 0;

  if (i < mN) {
    mLenCnt[(Depth < 16) ? Depth : 16]++;
  } else {
    Depth++;
    CountLen(mLeft [i]);
    CountLen(mRight[i]);
    Depth--;
  }
}

STATIC 
VOID 
MakeLen (
  IN INT32_EFI Root
  )
/*++

Routine Description:

  Create code length array for a Huffman tree
  
Arguments:

  Root   - the root of the tree

--*/
{
  INT32_EFI i, k;
  UINT32_EFI Cum;

  for (i = 0; i <= 16; i++) {
    mLenCnt[i] = 0;
  }
  CountLen(Root);
  
  //
  // Adjust the length count array so that
  // no code will be generated longer than its designated length
  //
  
  Cum = 0;
  for (i = 16; i > 0; i--) {
    Cum += mLenCnt[i] << (16 - i);
  }
  while (Cum != (1U << 16)) {
    mLenCnt[16]--;
    for (i = 15; i > 0; i--) {
      if (mLenCnt[i] != 0) {
        mLenCnt[i]--;
        mLenCnt[i+1] += 2;
        break;
      }
    }
    Cum--;
  }
  for (i = 16; i > 0; i--) {
    k = mLenCnt[i];
    while (--k >= 0) {
      mLen[*mSortPtr++] = (UINT8)i;
    }
  }
}

STATIC 
VOID 
DownHeap (
  IN INT32_EFI i
  )
{
  INT32_EFI j, k;

  //
  // priority queue: send i-th entry down heap
  //
  
  k = mHeap[i];
  while ((j = 2 * i) <= mHeapSize) {
    if (j < mHeapSize && mFreq[mHeap[j]] > mFreq[mHeap[j + 1]]) {
      j++;
    }
    if (mFreq[k] <= mFreq[mHeap[j]]) {
      break;
    }
    mHeap[i] = mHeap[j];
    i = j;
  }
  mHeap[i] = (INT16)k;
}

STATIC 
VOID 
MakeCode (
  IN  INT32_EFI n, 
  IN  UINT8 Len[], 
  OUT UINT16 Code[]
  )
/*++

Routine Description:

  Assign code to each symbol based on the code length array
  
Arguments:

  n     - number of symbols
  Len   - the code length array
  Code  - stores codes for each symbol

Returns: (VOID)

--*/
{
  INT32_EFI    i;
  UINT16   Start[18];

  Start[1] = 0;
  for (i = 1; i <= 16; i++) {
    Start[i + 1] = (UINT16)((Start[i] + mLenCnt[i]) << 1);
  }
  for (i = 0; i < n; i++) {
    Code[i] = Start[Len[i]]++;
  }
}

STATIC 
INT32_EFI 
MakeTree (
  IN  INT32_EFI   NParm, 
  IN  UINT16  FreqParm[], 
  OUT UINT8   LenParm[], 
  OUT UINT16  CodeParm[]
  )
/*++

Routine Description:

  Generates Huffman codes given a frequency distribution of symbols
  
Arguments:

  NParm    - number of symbols
  FreqParm - frequency of each symbol
  LenParm  - code length for each symbol
  CodeParm - code for each symbol
  
Returns:

  Root of the Huffman tree.
  
--*/
{
  INT32_EFI i, j, k, Avail;
  
  //
  // make tree, calculate len[], return root
  //

  mN = NParm;
  mFreq = FreqParm;
  mLen = LenParm;
  Avail = mN;
  mHeapSize = 0;
  mHeap[1] = 0;
  for (i = 0; i < mN; i++) {
    mLen[i] = 0;
    if (mFreq[i]) {
      mHeap[++mHeapSize] = (INT16)i;
    }    
  }
  if (mHeapSize < 2) {
    CodeParm[mHeap[1]] = 0;
    return mHeap[1];
  }
  for (i = mHeapSize / 2; i >= 1; i--) {
    
    //
    // make priority queue 
    //
    DownHeap(i);
  }
  mSortPtr = CodeParm;
  do {
    i = mHeap[1];
    if (i < mN) {
      *mSortPtr++ = (UINT16)i;
    }
    mHeap[1] = mHeap[mHeapSize--];
    DownHeap(1);
    j = mHeap[1];
    if (j < mN) {
      *mSortPtr++ = (UINT16)j;
    }
    k = Avail++;
    mFreq[k] = (UINT16)(mFreq[i] + mFreq[j]);
    mHeap[1] = (INT16)k;
    DownHeap(1);
    mLeft[k] = (UINT16)i;
    mRight[k] = (UINT16)j;
  } while (mHeapSize > 1);
  
  mSortPtr = CodeParm;
  MakeLen(k);
  MakeCode(NParm, LenParm, CodeParm);
  
  //
  // return root
  //
  return k;
}

STATIC
VOID
FillBuf (
  IN  SCRATCH_DATA  *Sd,
  IN  UINT16        NumOfBits
  )
/*++

Routine Description:

  Shift mBitBuf NumOfBits left. Read in NumOfBits of bits from source.

Arguments:

  Sd        - The global scratch data
  NumOfBits  - The number of bits to shift and read.

Returns: (VOID)

--*/
{
  Sd->mBitBuf = (UINT32_EFI) (Sd->mBitBuf << NumOfBits);

  while (NumOfBits > Sd->mBitCount) {

    Sd->mBitBuf |= (UINT32_EFI) (Sd->mSubBitBuf << (NumOfBits = (UINT16) (NumOfBits - Sd->mBitCount)));

    if (Sd->mCompSize > 0) {
      //
      // Get 1 byte into SubBitBuf
      //
      Sd->mCompSize--;
      Sd->mSubBitBuf  = 0;
      if (Sd->mSrcBase) {
        Sd->mSubBitBuf  = Sd->mSrcBase[Sd->mInBuf++];
      } else if (Sd->mIstrm) {
        Sd->mSubBitBuf  = Sd->mIstrm->get();
      }
      Sd->mBitCount   = 8;

    } else {
      //
      // No more bits from the source, just pad zero bit.
      //
      Sd->mSubBitBuf  = 0;
      Sd->mBitCount   = 8;

    }
  }

  Sd->mBitCount = (UINT16) (Sd->mBitCount - NumOfBits);
  Sd->mBitBuf |= Sd->mSubBitBuf >> Sd->mBitCount;

}

STATIC
UINT32_EFI
GetBits (
  IN  SCRATCH_DATA  *Sd,
  IN  UINT16        NumOfBits
  )
/*++

Routine Description:

  Get NumOfBits of bits out from mBitBuf. Fill mBitBuf with subsequent 
  NumOfBits of bits from source. Returns NumOfBits of bits that are 
  popped out.

Arguments:

  Sd            - The global scratch data.
  NumOfBits     - The number of bits to pop and read.

Returns:

  The bits that are popped out.

--*/
{
  UINT32_EFI  OutBits;

  OutBits = (UINT32_EFI) (Sd->mBitBuf >> (BITBUFSIZ - NumOfBits));

  FillBuf (Sd, NumOfBits);


  return OutBits;
}

STATIC
UINT16
MakeTable (
  IN  SCRATCH_DATA  *Sd,
  IN  UINT16        NumOfChar,
  IN  UINT8         *BitLen,
  IN  UINT16        TableBits,
  OUT UINT16        *Table
  )
/*++

Routine Description:

  Creates Huffman Code mapping table according to code length array.

Arguments:

  Sd        - The global scratch data
  NumOfChar - Number of symbols in the symbol set
  BitLen    - Code length array
  TableBits - The width of the mapping table
  Table     - The table
  
Returns:
  
  0         - OK.
  BAD_TABLE - The table is corrupted.

--*/
{
  UINT16  Count[17];
  UINT16  Weight[17];
  UINT16  Start[18];
  UINT16  *Pointer;
  UINT16  Index3;
  UINT16  Index;
  UINT16  Len;
  UINT16  Char;
  UINT16  JuBits;
  UINT16  Avail;
  UINT16  NextCode;
  UINT16  Mask;
  UINT16  Temp;

  for (Index = 1; Index <= 16; Index++) {
    Count[Index] = 0;
  }

  for (Index = 0; Index < NumOfChar; Index++) {
    Count[BitLen[Index]]++;
  }

  Start[1] = 0;

  for (Index = 1; Index <= 16; Index++) {
    Start[Index + 1] = (UINT16) (Start[Index] + (Count[Index] << (16 - Index)));
  }

  if (Start[17] != 0) {
    /*(1U << 16)*/
    return (UINT16) BAD_TABLE;
  }

  JuBits = (UINT16) (16 - TableBits);

  for (Index = 1; Index <= TableBits; Index++) {
    Start[Index] >>= JuBits;
    Weight[Index] = (UINT16) (1U << (TableBits - Index));
  }

  while (Index <= 16) {
    Temp = (UINT16) (1U << (16 - Index));
    Weight[Index++] = Temp;


//    Weight[Index++] = (UINT16)(1U << (16 - Index));


//  IN CASE OF FAIL REVERT.

  }

  Index = (UINT16) (Start[TableBits + 1] >> JuBits);

  if (Index != 0) {
    Index3 = (UINT16) (1U << TableBits);
    while (Index != Index3) {
      Table[Index++] = 0;
    }
  }

  Avail = NumOfChar;
  Mask  = (UINT16) (1U << (15 - TableBits));

  for (Char = 0; Char < NumOfChar; Char++) {

    Len = BitLen[Char];
    if (Len == 0) {
      continue;
    }

    NextCode = (UINT16) (Start[Len] + Weight[Len]);

    if (Len <= TableBits) {

      for (Index = Start[Len]; Index < NextCode; Index++) {
        Table[Index] = Char;
      }

    } else {

      Index3  = Start[Len];
      Pointer = &Table[Index3 >> JuBits];
      Index   = (UINT16) (Len - TableBits);

      while (Index != 0) {
        if (*Pointer == 0) {
          Sd->mRight[Avail]                     = Sd->mLeft[Avail] = 0;
          *Pointer = Avail++;
        }

        if (Index3 & Mask) {
          Pointer = &Sd->mRight[*Pointer];
        } else {
          Pointer = &Sd->mLeft[*Pointer];
        }

        Index3 <<= 1;
        Index--;
      }

      *Pointer = Char;

    }

    Start[Len] = NextCode;
  }
  //
  // Succeeds
  //
  return 0;
}

STATIC
UINT32_EFI
DecodeP (
  IN  SCRATCH_DATA  *Sd
  )
/*++

Routine Description:

  Decodes a position value.

Arguments:

  Sd      - the global scratch data

Returns:

  The position value decoded.

--*/
{
  UINT16  Val;
  UINT32_EFI  Mask;
  UINT32_EFI  Pos;

  Val = Sd->mPTTable[Sd->mBitBuf >> (BITBUFSIZ - 8)];

  if (Val >= MAXNP) {
    Mask = 1U << (BITBUFSIZ - 1 - 8);

    do {

      if (Sd->mBitBuf & Mask) {
        Val = Sd->mRight[Val];
      } else {
        Val = Sd->mLeft[Val];
      }

      Mask >>= 1;
    } while (Val >= MAXNP);
  }
  //
  // Advance what we have read
  //
  FillBuf (Sd, Sd->mPTLen[Val]);

  Pos = Val;

  if (Val > 1) {
    Pos = (UINT32_EFI) ((1U << (Val - 1)) + GetBits (Sd, (UINT16) (Val - 1)));
  }


  return Pos;
}

STATIC
UINT16
ReadPTLen (
  IN  SCRATCH_DATA  *Sd,
  IN  UINT16        nn,
  IN  UINT16        nbit,
  IN  UINT16        Special
  )
/*++

Routine Description:

  Reads code lengths for the Extra Set or the Position Set

Arguments:

  Sd        - The global scratch data
  nn        - Number of symbols
  nbit      - Number of bits needed to represent nn
  Special   - The special symbol that needs to be taken care of 

Returns:

  0         - OK.
  BAD_TABLE - Table is corrupted.

--*/
{
  UINT16  Number;
  UINT16  CharC;
  UINT16  Index;
  UINT32_EFI  Mask;

  Number = (UINT16) GetBits (Sd, nbit);

  if (Number == 0) {
    CharC = (UINT16) GetBits (Sd, nbit);

    for (Index = 0; Index < 256; Index++) {
      Sd->mPTTable[Index] = CharC;
    }

    for (Index = 0; Index < nn; Index++) {
      Sd->mPTLen[Index] = 0;
    }

    return 0;
  }

  Index = 0;

  while (Index < Number) {

    CharC = (UINT16) (Sd->mBitBuf >> (BITBUFSIZ - 3));

    if (CharC == 7) {
      Mask = 1U << (BITBUFSIZ - 1 - 3);
      while (Mask & Sd->mBitBuf) {
        Mask >>= 1;
        CharC += 1;
      }
    }

    FillBuf (Sd, (UINT16) ((CharC < 7) ? 3 : CharC - 3));

    Sd->mPTLen[Index++] = (UINT8) CharC;

    if (Index == Special) {
      CharC = (UINT16) GetBits (Sd, 2);
      while ((INT16) (--CharC) >= 0) {
        Sd->mPTLen[Index++] = 0;
      }
    }
  }

  while (Index < nn) {
    Sd->mPTLen[Index++] = 0;
  }

  return MakeTable (Sd, nn, Sd->mPTLen, 8, Sd->mPTTable);
}

STATIC
VOID
ReadCLen (
  SCRATCH_DATA  *Sd
  )
/*++

Routine Description:

  Reads code lengths for Char&Len Set.

Arguments:

  Sd    - the global scratch data

Returns: (VOID)

--*/
{
  UINT16  Number;
  UINT16  CharC;
  UINT16  Index;
  UINT32_EFI  Mask;

  Number = (UINT16) GetBits (Sd, CBIT);

  if (Number == 0) {
    CharC = (UINT16) GetBits (Sd, CBIT);

    for (Index = 0; Index < NC; Index++) {
      Sd->mCLen[Index] = 0;
    }

    for (Index = 0; Index < 4096; Index++) {
      Sd->mCTable[Index] = CharC;
    }

    return ;
  }

  Index = 0;
  while (Index < Number) {

    CharC = Sd->mPTTable[Sd->mBitBuf >> (BITBUFSIZ - 8)];
    if (CharC >= NT) {
      Mask = 1U << (BITBUFSIZ - 1 - 8);

      do {

        if (Mask & Sd->mBitBuf) {
          CharC = Sd->mRight[CharC];
        } else {
          CharC = Sd->mLeft[CharC];
        }

        Mask >>= 1;

      } while (CharC >= NT);
    }
    //
    // Advance what we have read
    //
    FillBuf (Sd, Sd->mPTLen[CharC]);

    if (CharC <= 2) {

      if (CharC == 0) {
        CharC = 1;
      } else if (CharC == 1) {
        CharC = (UINT16) (GetBits (Sd, 4) + 3);
      } else if (CharC == 2) {
        CharC = (UINT16) (GetBits (Sd, CBIT) + 20);
      }

      while ((INT16) (--CharC) >= 0) {
        Sd->mCLen[Index++] = 0;
      }

    } else {

      Sd->mCLen[Index++] = (UINT8) (CharC - 2);

    }
  }

  while (Index < NC) {
    Sd->mCLen[Index++] = 0;
  }

  MakeTable (Sd, NC, Sd->mCLen, 12, Sd->mCTable);

  return ;
}

STATIC
UINT16
DecodeC (
  SCRATCH_DATA  *Sd
  )
/*++

Routine Description:

  Decode a character/length value.

Arguments:

  Sd    - The global scratch data.

Returns:

  The value decoded.

--*/
{
  UINT16  Index2;
  UINT32_EFI  Mask;

  if (Sd->mBlockSize == 0) {
    //
    // Starting a new block
    //

    Sd->mBlockSize    = (UINT16) GetBits (Sd, 16);
    Sd->mBadTableFlag = ReadPTLen (Sd, NT, TBIT, 3);

    if (Sd->mBadTableFlag != 0) {
      return 0;
    }

    ReadCLen (Sd);

    Sd->mBadTableFlag = ReadPTLen (Sd, MAXNP, Sd->mPBit, (UINT16) (-1));

    if (Sd->mBadTableFlag != 0) {
      return 0;
    }
  }

  Sd->mBlockSize--;
  Index2 = Sd->mCTable[Sd->mBitBuf >> (BITBUFSIZ - 12)];


  if (Index2 >= NC) {

    Mask = 1U << (BITBUFSIZ - 1 - 12);

    do {

      if (Sd->mBitBuf & Mask) {
        Index2 = Sd->mRight[Index2];
      } else {
        Index2 = Sd->mLeft[Index2];
      }

      Mask >>= 1;
    } while (Index2 >= NC);

  }

  //
  // Advance what we have read
  //

  FillBuf (Sd, Sd->mCLen[Index2]);

  return Index2;
}

STATIC
UINT32_EFI
MoveWindow(SCRATCH_DATA* Sd, UINT32_EFI idx)
{
  if (idx == WNDSIZ * 2) {
    memmove(&(Sd->mOstrmWin[0]), &(Sd->mOstrmWin[WNDSIZ]), WNDSIZ + MAXMATCH);
    idx = WNDSIZ;
    Sd->mOstrmWinStartidx += WNDSIZ;
  }
  return idx;
}

STATIC
VOID
Decode (
  SCRATCH_DATA  *Sd
  )
/*++

Routine Description:

  Decode the source data and put the resulting data into the destination buffer.

Arguments:

  Sd            - The global scratch data

Returns: (VOID)

 --*/
{
  UINT16  BytesRemain;
  UINT32_EFI  DataIdx;
  UINT32_EFI  winidx;
  UINT16  CharC;

  BytesRemain = (UINT16) (-1);

  DataIdx     = 0;
  winidx      = 0;

  for (;;) {

    CharC = DecodeC (Sd);
    if (Sd->mBadTableFlag != 0) {
      return ;
    }

    if (CharC < 256) {
      //
      // Process an Original character
      //
      if (Sd->mOutBuf >= Sd->mOrigSize) {
        return ;
      } else {
        if (Sd->mDstBase) {
          Sd->mDstBase[Sd->mOutBuf++] = (UINT8) CharC;
        } else if (Sd->mOstrm) {
          // write char to output stream and keep a copy in window
          char data = (char) CharC;
          Sd->mOstrm->put(data);
          Sd->mOstrmWin[winidx++] = data;
          Sd->mOutBuf++;
          winidx = MoveWindow(Sd, winidx);
        }
      }

    } else {
      //
      // Process a Pointer
      //
      CharC       = (UINT16) (CharC - (UINT8_MAX + 1 - THRESHOLD));

      BytesRemain = CharC;

      DataIdx     = Sd->mOutBuf - DecodeP (Sd) - 1;

      BytesRemain--;

      while ((INT16) (BytesRemain) >= 0) {
        if (Sd->mDstBase) {
          Sd->mDstBase[Sd->mOutBuf++] = Sd->mDstBase[DataIdx++];
        } else if (Sd->mOstrm) {
          int windataidx = DataIdx - Sd->mOstrmWinStartidx;
          if (windataidx >= 0 && 
              (UINT32_EFI) windataidx <= winidx ) {
            // write char to output stream and keep a copy in window
            Sd->mOstrm->put( Sd->mOstrmWin[windataidx] );
            Sd->mOstrmWin[winidx] = Sd->mOstrmWin[windataidx];

            DataIdx++;
            winidx++;
            // update window 
            winidx = MoveWindow(Sd, winidx);
          } else
          {
              fprintf(stderr, "error: win data idx is bad: %d=%d-%d, %d-%d=%d\n", windataidx, DataIdx, Sd->mOstrmWinStartidx, winidx, BytesRemain, winidx-BytesRemain);
          }
          Sd->mOutBuf++;
        }
        if (Sd->mOutBuf >= Sd->mOrigSize) {
          return ;
        }

        BytesRemain--;
      }
    }
  }

  return ;
}

EFI_STATUS
GetInfo (
  IN      VOID    *Source,
  IN      UINT32_EFI  SrcSize,
  OUT     UINT32_EFI  *DstSize,
  OUT     UINT32_EFI  *ScratchSize
  )
/*++

Routine Description:

  The internal implementation of *_DECOMPRESS_PROTOCOL.GetInfo().

Arguments:

  Source      - The source buffer containing the compressed data.
  SrcSize     - The size of source buffer
  DstSize     - The size of destination buffer.
  ScratchSize - The size of scratch buffer.

Returns:

  EFI_SUCCESS           - The size of destination buffer and the size of scratch buffer are successull retrieved.
  EFI_INVALID_PARAMETER - The source data is corrupted

--*/
{
  UINT8 *Src;

  *ScratchSize  = sizeof (SCRATCH_DATA);

  Src           = (UINT8 *) Source;
  if (SrcSize < 8) {
    return EFI_INVALID_PARAMETER;
  }

  *DstSize = Src[4] + (Src[5] << 8) + (Src[6] << 16) + (Src[7] << 24);
  return EFI_SUCCESS;
}

EFI_STATUS
GetInfo (
  IN      std::istream& istrm,
  OUT     UINT32_EFI  *DstSize,
  OUT     UINT32_EFI  *ScratchSize
  )
/*++

Routine Description:

  The internal implementation of *_DECOMPRESS_PROTOCOL.GetInfo().

Arguments:

  Source      - The source stream containing the compressed data.
  DstSize     - The size of destination buffer.
  ScratchSize - The size of scratch buffer.

Returns:

  EFI_SUCCESS           - The size of destination buffer and the size of scratch buffer are successull retrieved.
  EFI_INVALID_PARAMETER - The source data is corrupted

--*/
{
  EFI_STATUS rc = EFI_SUCCESS;
  UINT32_EFI SrcSize;

  *ScratchSize  = sizeof (SCRATCH_DATA);
  SrcSize       = GetDword(&istrm, 0);
  *DstSize      = GetDword(&istrm, 4);

  if (istrm.eof())
    rc = EFI_INVALID_PARAMETER;     // file too short

  istrm.seekg(0, istrm.end);
  unsigned filelen = istrm.tellg();

  if (filelen != SrcSize + 8) {
    rc = EFI_INVALID_PARAMETER;     // incorrect format
  }

  istrm.clear();
  istrm.seekg(0);

  return rc;
}

EFI_STATUS
Decompress (
  IN      VOID    *Source,
  IN      UINT32_EFI  SrcSize,
  IN OUT  VOID    *Destination,
  IN      UINT32_EFI  DstSize,
  IN      std::istream* istrm,
  IN OUT  std::ostream* ostrm,
  IN OUT  VOID    *Scratch,
  IN      UINT32_EFI  ScratchSize,
  IN      UINT8   Version
  )
/*++

Routine Description:

  The internal implementation of *_DECOMPRESS_PROTOCOL.Decompress().

Arguments:

  Source      - The source buffer containing the compressed data.
  SrcSize     - The size of source buffer
  Destination - The destination buffer to store the decompressed data
  DstSize     - The size of destination buffer.
  ostrm       - The output stream (optional, instead of Destination buffer)
  Scratch     - The buffer used internally by the decompress routine. This  buffer is needed to store intermediate data.
  ScratchSize - The size of scratch buffer.
  Version     - The version of de/compression algorithm.
                Version 1 for EFI 1.1 de/compression algorithm.
                Version 2 for Tiano de/compression algorithm.

Returns:

  EFI_SUCCESS           - Decompression is successfull
  EFI_INVALID_PARAMETER - The source data is corrupted

--*/
{

  UINT32_EFI        Index;
  UINT32_EFI        CompSize = 0;
  UINT32_EFI        OrigSize = 0;
  EFI_STATUS    Status;
  SCRATCH_DATA  *Sd;
  UINT8         *Src;
  UINT8         *Dst;

  Status  = EFI_SUCCESS;
  Src     = (UINT8*) Source;
  Dst     = (UINT8*) Destination;

  if (ScratchSize < sizeof (SCRATCH_DATA)) {
    return EFI_INVALID_PARAMETER;
  }

  Sd = (SCRATCH_DATA *) Scratch;

  if (Src && SrcSize < 8) {
    return EFI_INVALID_PARAMETER;
  }

  if (Src) {
    CompSize  = GetDword(Src,0);
    OrigSize  = GetDword(Src,4);
  } else if (istrm) {
    CompSize  = GetDword(istrm,0);
    OrigSize  = GetDword(istrm,4);
  }

  //
  // If compressed file size is 0, return
  //
  if (OrigSize == 0) {
    return Status;
  }

  // get length of input stream
  if (istrm) {
    istrm->seekg(0, istrm->end);
    SrcSize = istrm->tellg();
    istrm->seekg(8);
  }

  if (SrcSize < CompSize + 8) {
    return EFI_INVALID_PARAMETER;
  }

  if (Dst && DstSize != OrigSize) {
    return EFI_INVALID_PARAMETER;
  }

  if (Src)
    Src = Src + 8;

  for (Index = 0; Index < sizeof (SCRATCH_DATA); Index++) {
    ((UINT8 *) Sd)[Index] = 0;
  }
  
  UINT8* ostrm_window = 0;
  if (ostrm) {
    ostrm_window = (UINT8*) calloc (WNDSIZ * 2 + MAXMATCH, 1);
  }

  //
  // The length of the field 'Position Set Code Length Array Size' in Block Header.
  // For EFI 1.1 de/compression algorithm(Version 1), mPBit = 4
  // For Tiano de/compression algorithm(Version 2), mPBit = 5
  //
  switch (Version) {
  case 1:
    Sd->mPBit = 4;
    break;

  case 2:
    Sd->mPBit = 5;
    break;

  default:
    //
    // Currently, only have 2 versions
    //
    return EFI_INVALID_PARAMETER;
  }

  Sd->mSrcBase  = Src;
  Sd->mDstBase  = Dst;
  Sd->mIstrm    = istrm;
  Sd->mOstrm    = ostrm;
  Sd->mOstrmWin = ostrm_window;
  Sd->mCompSize = CompSize;
  Sd->mOrigSize = OrigSize;

  //
  // Fill the first BITBUFSIZ bits
  //

//  FillBuf (Sd, BITBUFSIZ - 1);
    FillBuf(Sd, BITBUFSIZ);
  //
  // Decompress it
  //
  Decode (Sd);

  if (ostrm_window) {
    free(ostrm_window);
  }

  if (Sd->mBadTableFlag != 0) {
    //
    // Something wrong with the source
    //
    Status = EFI_INVALID_PARAMETER;
  }

  return Status;
}

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
{
  return GetInfo (
          Source,
          SrcSize,
          DstSize,
          ScratchSize
          );
}

EFI_STATUS
EFIAPI
EfiGetInfo (
  IN      std::istream&  istrm,
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
{
  return GetInfo (
          istrm,
          DstSize,
          ScratchSize
          );
}

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

  The implementation is same as that of EFI_DECOMPRESS_PROTOCOL.Decompress().

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
{
  //
  // For EFI 1.1 de/compression algorithm, the version is 1.
  //
  return Decompress (
          Source,
          SrcSize,
          Destination,
          DstSize,
          0,
          0,
          Scratch,
          ScratchSize,
          2
          );
}

EFI_STATUS
EFIAPI
EfiDecompress (
  IN      VOID                    *Source,
  IN      UINT32_EFI                  SrcSize,
  IN OUT  std::ostream&            ostrm,
  IN OUT  VOID                    *Scratch,
  IN      UINT32_EFI                  ScratchSize
  )
/*++

Routine Description:

  The implementation is same as that of EFI_DECOMPRESS_PROTOCOL.Decompress().

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
{
  //
  // For EFI 1.1 de/compression algorithm, the version is 1.
  //
  return Decompress(
          Source,
          SrcSize,
          0,
          0,
          0,
          &ostrm,
          Scratch,
          ScratchSize,
          2
          );
}

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

  The implementation is same as that of EFI_DECOMPRESS_PROTOCOL.Decompress().

Arguments:

  This        - The protocol instance pointer
  istrm       - The input stream containing the compressed data.
  ostrm       - The output stream to contain the decompressed data
  Scratch     - The buffer used internally by the decompress routine. This  buffer is needed to store intermediate data.
  ScratchSize - The size of scratch buffer.

Returns:

  EFI_SUCCESS           - Decompression is successfull
  EFI_INVALID_PARAMETER - The source data is corrupted

--*/
{
  //
  // For EFI 1.1 de/compression algorithm, the version is 1.
  //
  return Decompress(
          0,
          0,
          0,
          0,
          &istrm,
          &ostrm,
          Scratch,
          ScratchSize,
          2
          );
}
