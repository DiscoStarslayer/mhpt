// MakeMD5.h: CMakeMD5 クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "MakeHashAlgorithm.h"

class CMakeMD5 : public CMakeHashAlgorithm
{
public:
    CMakeMD5();
    ~CMakeMD5();

    virtual char* GetHashName(char *pBuffer=NULL) { return (pBuffer) ? strcpy(pBuffer,"MD5") : "MD5"; }
    virtual DWORD GetHashValueSize() { return 16; }
    virtual DWORD GetHashBlockSize() { return 64; }
    virtual void Initialize();

    virtual BOOL Test();

private:
    virtual DWORD ComputeHashHelper(const void *pBuffer, DWORD len)
            { return ComputeHashHelper(pBuffer, m_pHash, len); }
    virtual DWORD ComputeHashHelper(const void *pBuffer, void *msg_digest, DWORD len);
    virtual void ComputeHashFinalHelper(void *msg_digest);

private:
    DWORD m_pHash[4];
};

