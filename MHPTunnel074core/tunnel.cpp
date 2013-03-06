//---------------------------------------------------------------------------

#include <windows.h>
//---------------------------------------------------------------------------
// The following are the memory management when you create a DLL that does not use shared RTL DLL
// It is a note about
//
// Structure that contains and AnsiString (AnsiString as parameters and return values ??/
// If you want to export from the DLL functions to handle the class), and that DLL, DLL
// You need to add the library to both MEMMGR.LIB project to use
// Some.
//
// Has been exported from a DLL, the class that is not derived from TObject
// Do not add MEMMGR.LIB even if you use new or delete for
// I must.
//
// By adding MEMMGR.LIB, EXE that references the DLL and the DLL is both
// I would like to use the memory manager of the communication. The memory manager BORLNDMM.DLL
// Are provided as. DLL or to distribute with your application
// Please.
//
// To avoid BORLNDMM.DLL from being used, instead of the AnsiString type
// Perform the interaction of the string using a ShortString or type "char *"
// Please
//
// If the DLL that you create using the RTL DLL shared, MEMMGR.LIB Toward the RTL
// Explicitly added to the DLL project as a library for adding
// Does not need to.
//---------------------------------------------------------------------------

#pragma argsused
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
	return 1;
}
//---------------------------------------------------------------------------
