// IntegrityCheck.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <wchar.h>
#include <xbdm.h>

wchar_t buffer[0x100];
wchar_t * buttonText = L"OK";
XOVERLAPPED theXOverlap;
MESSAGEBOX_RESULT theUIResult;

HRESULT returnResult;
DWORD WriteMemory(LPVOID address, DWORD size, LPCVOID data, HRESULT * optionalOutput)
{
	DWORD outInt;
#ifdef _DEBUG
	returnResult = DmSetMemory(address, size, data, &outInt);
#endif
#ifdef NDEBUG
	returnResult = memcpy(address, data, size) == address ? ERROR_SUCCESS : TYPE_E_UNDEFINEDTYPE;
	outInt = size;
#endif

	if(optionalOutput != 0)
		optionalOutput = &returnResult;

	return outInt;
}

extern "C" VOID DbgPrint(const char* s, ...);
int __cdecl main()
{	
	//RETAIL DISC LOADING:
	//0x800C4FA0
	//0x800C4FC8
	//to  li %r11, 1
	int loadImidiate1Tor11 = 0x39600001;
	int messageBoxReturn;
	bool aborted = false;
	int percentage = 0;
    XSECURITY_FAILURE_INFORMATION FailureInformation = {0};
    FailureInformation.dwSize = sizeof( XSECURITY_FAILURE_INFORMATION );

	WriteMemory((LPVOID)0x800C4FA0, 4, &loadImidiate1Tor11, 0);
	WriteMemory((LPVOID)0x800C4FC8, 4, &loadImidiate1Tor11, 0);

	messageBoxReturn = XShowMessageBoxUI(0, L"Integrity Check", L"This will check the integrity of the disc using ECC code.\nDo NOT eject the disc without exiting first!\n\nPress B to exit at any time\n", 1, (LPCWSTR *)&buttonText, 0, XMB_NOICON, &theUIResult, &theXOverlap);

    DWORD dwResult = XSecurityCreateProcess( 4 ); // Use hardware thread 4
    if( dwResult != ERROR_DISK_CORRUPT ) {
		while(!FailureInformation.fComplete && !aborted) {
			XINPUT_STATE currentState;

			if(XInputGetState(0, &currentState) == ERROR_SUCCESS)
				if(currentState.Gamepad.wButtons & XINPUT_GAMEPAD_B) {
					aborted = true;
					break;
				}
			
			dwResult = XSecurityVerify( 0,NULL,NULL );
			if( dwResult != ERROR_SUCCESS )
				continue;

			dwResult = XSecurityGetFailureInfo( &FailureInformation );

			int temp_percentage = (int)(((float)FailureInformation.dwBlocksChecked / FailureInformation.dwTotalBlocks) * 100);
			if(temp_percentage != percentage) {
				percentage = temp_percentage;
				DbgPrint("%i%% complete.", percentage);
				if(FailureInformation.dwFailedHashes)
					DbgPrint(" %i failed hashes!", FailureInformation.dwFailedHashes);
				if(FailureInformation.dwFailedReads)
					DbgPrint(" %i failed reads!", FailureInformation.dwFailedReads);
				DbgPrint("\n");
			}
		}
		
		dwResult = XSecurityGetFailureInfo( &FailureInformation );
		XSecurityCloseProcess();
	}
	else
		aborted = true;

	//A non-zero value of dwFailedHashes always means an integrity failure. 
	//If dwFailedReads is greater than 10% of dwBlocksChecked, then an integrity error is recommended. Clean the disc and try again.
	if( FailureInformation.dwFailedHashes > 0 || FailureInformation.dwFailedReads > (FailureInformation.dwBlocksChecked * 0.1) || aborted)
		swprintf(buffer, L"Integrity Check Failed!\n\nBlocks Checked: %i/%i\nFailed Hashes: %i\nFailed Reads: %i", FailureInformation.dwBlocksChecked, FailureInformation.dwTotalBlocks, FailureInformation.dwFailedHashes, FailureInformation.dwFailedReads);
	else
		swprintf(buffer, L"Integrity Check Succeeded!\n\nBlocks Checked: %i/%i\nFailed Hashes: %i\nFailed Reads: %i", FailureInformation.dwBlocksChecked, FailureInformation.dwTotalBlocks, FailureInformation.dwFailedHashes, FailureInformation.dwFailedReads);
		
	ZeroMemory(&theXOverlap, sizeof(XOVERLAPPED));
	messageBoxReturn = XShowMessageBoxUI(0, L"Integrity Check", buffer, 1, (LPCWSTR *)&buttonText, 0, XMB_NOICON, &theUIResult, &theXOverlap);
	while(!XHasOverlappedIoCompleted(&theXOverlap)) { Sleep(0); }

	return 0;
}

