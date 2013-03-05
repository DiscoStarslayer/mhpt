#include "stdafx.h"
#include "MakeHashAlgorithm.h"
#include <ctype.h>


static int ReadHashValue(void *hashbuf, const char *buf);

static DWORD atox_table[256] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,  0x08, 0x09, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0xff,  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0xff,  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};


////////////////////////////////////////////////////////////////////////////////////////////////////

CMakeHashAlgorithm::CMakeHashAlgorithm()
{
}

CMakeHashAlgorithm::~CMakeHashAlgorithm()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CMakeHashAlgorithm::Initialize()
{
    m_qwSrcLen  = 0;
    m_dwRestLen = 0;
}

void CMakeHashAlgorithm::ComputeHash(const void *pBuffer, DWORD len)
{
    if ( len == 0 )
        return;

    DWORD tmp, restlen=m_dwRestLen;
    m_qwSrcLen += len;

    if ( restlen > 0 ) {
        DWORD dwHashBlockSize = GetHashBlockSize();
        if ( restlen+len < dwHashBlockSize ) {
            m_dwRestLen = restlen + len;
            memcpy(&m_pRest[restlen], pBuffer, len);
            return;
        } else {
            tmp = dwHashBlockSize - restlen;
            memcpy(&m_pRest[restlen], pBuffer, tmp);
            ComputeHashHelper(m_pRest, dwHashBlockSize);
            m_dwRestLen = 0;
            len -= tmp;
            pBuffer = (BYTE*)pBuffer + tmp;
            if ( len == 0 )
                return;
        }
    }

    restlen = ComputeHashHelper(pBuffer, len);
    if ( restlen > 0 ) {
        memcpy(m_pRest, (BYTE*)pBuffer+len-restlen, restlen);
        m_dwRestLen = restlen;
    }
}

void CMakeHashAlgorithm::GetHashValue(void *msg_digest)
{
    ComputeHashFinalHelper(msg_digest);
    Initialize();
}

void CMakeHashAlgorithm::GetHashValueWithoutInitializing(void *msg_digest)
{
    ComputeHashFinalHelper(msg_digest);
}

void CMakeHashAlgorithm::GetHashString(char *pStrBuf, BOOL bSmallString)
{
    static const char *xstring_s = "0123456789abcdef";
    static const char *xstring_b = "0123456789ABCDEF";
    const char *xstring = ( bSmallString ) ? xstring_s : xstring_b;
    DWORD i, size = GetHashValueSize();
    BYTE hash[1024];

    GetHashValue(hash);

    for (i=0; i<size; i++) {
        *(pStrBuf+0) = xstring[hash[i]>>4];
        *(pStrBuf+1) = xstring[hash[i]&0x0f];
        pStrBuf += 2;
    }
    *pStrBuf = '\0';
}

int CMakeHashAlgorithm::CmpHashValue(const char *pHashStr)
{
    BYTE v1[1024], v2[1024];
    DWORD size = GetHashValueSize();
    if ( ReadHashValue(v2, pHashStr) != size )
        return -10000;
    GetHashValue(v1);
    return memcmp(v1, v2, size);
}

int ReadHashValue(void *hashbuf, const char *buf)
{
    BYTE *pHash = (BYTE*)hashbuf;
    DWORD tmp1, tmp2;
    int cnt=0;

    while ( *buf!='\0' ) {
        tmp1 = atox_table[*buf];
        tmp2 = atox_table[*(buf+1)];
        if ( tmp1==0xff || tmp2==0xff )
            break;
        *pHash = (BYTE)(tmp2 | (tmp1<<4));
        pHash++;
        buf+=2;
        cnt++;
        if ( cnt >= 1024 )
            break;
    }
    return cnt;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// ReverseEndian

#if (USE_ASSEMBLER_REVERSEENDIAN==0)

void ReverseEndian32(void *dest, const void *src, DWORD len)
{
    DWORD dwData;
    DWORD *pDest = (DWORD*)dest;
    DWORD *pSrc = (DWORD*)src;
    len /= 4;

    while ( len > 0 ) {
        dwData = *pSrc++;
        *pDest++ = (dwData << 24) | ((dwData & 0x0000ff00) << 8) | ((dwData & 0x00ff0000) >> 8) | (dwData >> 24);
        len--;
    }
}

void ReverseEndian64(void *dest, const void *src, DWORD len)
{
    DWORD dwData1, dwData2;
    DWORD *pDest = (DWORD*)dest;
    DWORD *pSrc = (DWORD*)src;
    len /= 8;

    while ( len > 0 ) {
        dwData1 = *pSrc;
        dwData2 = *(pSrc+1);
        *(pDest+1) = (dwData1 << 24) | ((dwData1 & 0x0000ff00) << 8) | ((dwData1 & 0x00ff0000) >> 8) | (dwData1 >> 24);
        *(pDest) = (dwData2 << 24) | ((dwData2 & 0x0000ff00) << 8) | ((dwData2 & 0x00ff0000) >> 8) | (dwData2 >> 24);
        pSrc+=2;
        pDest+=2;
        len--;
    }
}

#endif  // (USE_REVERSEENDIAN_ASSEMBLER==0)

////////////////////////////////////////////////////////////////////////////////////////////////////
