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

//#define CDECL __cdecl   // If this line of definition VC
#define CDECL           // Otherwise VC either defined this line, redefine as appropriate

// ROTATE
//#define ROTATE_L32(x,n)     ( _rotl(x,n) )  // If you can () _rotl has priority here
//#define ROTATE_R32(x,n)     ( _rotr(x,n) )  // If you can () _rotr has priority here
#define ROTATE_L32(x,n)     ( ((x) << (n)) | ((x) >> (32 - (n))) )  // If you can not use () is used here _rotl
#define ROTATE_R32(x,n)     ( ((x) >> (n)) | ((x) << (32 - (n))) )  // If you can not use () is used here _rotr


// ReverseEndian If you use the assembler version in 1 If you do not use 0
#define USE_REVERSEENDIAN_ASSEMBLER 0

//If you want to use in each hash assembler version 1 If you do not use 0
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

