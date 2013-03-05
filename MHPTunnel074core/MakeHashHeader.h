#pragma once

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define ASSERT assert
typedef int BOOL;
#define TRUE 1
#define FALSE 0
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned __int64 QWORD;

//#define CDECL __cdecl   // VCの場合はこの行を定義
#define CDECL           // VC以外の場合はこの行を定義するか、適宜定義し直す

// ROTATE
//#define ROTATE_L32(x,n)     ( _rotl(x,n) )  // _rotl()が使える場合はこちらを優先
//#define ROTATE_R32(x,n)     ( _rotr(x,n) )  // _rotr()が使える場合はこちらを優先
#define ROTATE_L32(x,n)     ( ((x) << (n)) | ((x) >> (32 - (n))) )  // _rotl()が使えない場合はこちらを使う
#define ROTATE_R32(x,n)     ( ((x) >> (n)) | ((x) << (32 - (n))) )  // _rotr()が使えない場合はこちらを使う


// ReverseEndianでアセンブラ版を使う場合は1、使わない場合は0
#define USE_REVERSEENDIAN_ASSEMBLER 0

// 各ハッシュでアセンブラ版を使う場合は1、使わない場合は0
#define USE_ASSEMBLER_CRC           0
#define USE_ASSEMBLER_MD5           0
#define USE_ASSEMBLER_SHA1          0
#define USE_ASSEMBLER_SHA256        0
#define USE_ASSEMBLER_SHA384        0
#define USE_ASSEMBLER_SHA512        0
#define USE_ASSEMBLER_RIPEMD128     0
#define USE_ASSEMBLER_RIPEMD160     0
#define USE_ASSEMBLER_RIPEMD256     0
#define USE_ASSEMBLER_RIPEMD320     0


#if (USE_REVERSEENDIAN_ASSEMBLER==0)
    void ReverseEndian32(void *dest, const void *src, DWORD len);
    void ReverseEndian64(void *dest, const void *src, DWORD len);
#else
    extern "C" void CDECL ReverseEndian32(void *dest, const void *src, DWORD len);
    extern "C" void CDECL ReverseEndian64(void *dest, const void *src, DWORD len);
#endif

