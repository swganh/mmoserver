///////////////////////////////////////////////////////////////////////////////
// Include files 
//
#ifndef SRC_UTILS_MDUMP_H_
#define SRC_UTILS_MDUMP_H_

#include <windows.h>
#include <tchar.h>
#include <dbghelp.h>
#include <stdio.h>
#include <crtdbg.h>


///////////////////////////////////////////////////////////////////////////////
// Directives 
//

#pragma comment ( lib, "dbghelp.lib" )


///////////////////////////////////////////////////////////////////////////////
// Function declarations 
//

LONG WINAPI CreateMiniDump( LPEXCEPTION_POINTERS pep ); 

BOOL CALLBACK MiniDumpCallback(
	PVOID                            pParam, 
	const PMINIDUMP_CALLBACK_INPUT   pInput, 
	PMINIDUMP_CALLBACK_OUTPUT        pOutput 
); 

#endif
