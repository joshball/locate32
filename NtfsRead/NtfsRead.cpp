// NtfsRead.cpp : Defines the entry point for the console application.
//

#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>

//
// File System Control command for getting NTFS information
//
#define FSCTL_GET_VOLUME_INFORMATION	0x90064


//
// return code type
//
typedef UINT NTSTATUS;

//
// Error codes returned by NtFsControlFile (see NTSTATUS.H)
//
#define STATUS_SUCCESS			         ((NTSTATUS)0x00000000L)
#define STATUS_BUFFER_OVERFLOW           ((NTSTATUS)0x80000005L)
#define STATUS_INVALID_PARAMETER         ((NTSTATUS)0xC000000DL)
#define STATUS_BUFFER_TOO_SMALL          ((NTSTATUS)0xC0000023L)
#define STATUS_ALREADY_COMMITTED         ((NTSTATUS)0xC0000021L)
#define STATUS_INVALID_DEVICE_REQUEST    ((NTSTATUS)0xC0000010L)

//
// Io Status block (see NTDDK.H)
//
typedef struct _IO_STATUS_BLOCK {
    NTSTATUS Status;
    ULONG Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;


//
// Apc Routine (see NTDDK.H)
//
typedef VOID (*PIO_APC_ROUTINE) (
				PVOID ApcContext,
				PIO_STATUS_BLOCK IoStatusBlock,
				ULONG Reserved
			);

//
// The undocumented NtFsControlFile
//
// This function is used to send File System Control (FSCTL)
// commands into file system drivers. Its definition is 
// in ntdll.dll (ntdll.lib), a file shipped with the NTDDK.
//
NTSTATUS (__stdcall *NtFsControlFile)( 
					HANDLE FileHandle,
					HANDLE Event,					// optional
					PIO_APC_ROUTINE ApcRoutine,		// optional
					PVOID ApcContext,				// optional
					PIO_STATUS_BLOCK IoStatusBlock,	
					ULONG FsControlCode,
					PVOID InputBuffer,				// optional
					ULONG InputBufferLength,
					PVOID OutputBuffer,				// optional
					ULONG OutputBufferLength
			);



//
// NTFS volume information
//
typedef struct {
	LARGE_INTEGER    	SerialNumber;
	LARGE_INTEGER    	NumberOfSectors;
	LARGE_INTEGER    	TotalClusters;
	LARGE_INTEGER    	FreeClusters;
	LARGE_INTEGER    	Reserved;
	ULONG    			BytesPerSector;
	ULONG    			BytesPerCluster;
	ULONG    			BytesPerMFTRecord;
	ULONG    			ClustersPerMFTRecord;
	LARGE_INTEGER    	MFTLength;
	LARGE_INTEGER    	MFTStart;
	LARGE_INTEGER    	MFTMirrorStart;
	LARGE_INTEGER    	MFTZoneStart;
	LARGE_INTEGER    	MFTZoneEnd;
} NTFS_VOLUME_DATA_BUFFER, *PNTFS_VOLUME_DATA_BUFFER;


//--------------------------------------------------------------------
//
// GetNTFSInfo
//
// Open the volume and query its data.
//
//--------------------------------------------------------------------
BOOLEAN GetNTFSInfo( int DriveId, PNTFS_VOLUME_DATA_BUFFER VolumeInfo ) 
{
	static char			volumeName[] = "\\\\.\\A:";
	HANDLE				volumeHandle;
	IO_STATUS_BLOCK		ioStatus;
	NTSTATUS			status;

	//
	// open the volume
	//
	volumeName[4] = DriveId + 'A'; 
	volumeHandle = CreateFile( volumeName, GENERIC_READ, 
					FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 
					0, 0 );
	if( volumeHandle == INVALID_HANDLE_VALUE )	{

		printf("\nError opening volume: ");
		//PrintWin32Error( GetLastError() );
		return FALSE;
	}

	//
	// Query the volume information
	//
	status = NtFsControlFile( volumeHandle, NULL, NULL, 0, &ioStatus,
						FSCTL_GET_VOLUME_INFORMATION,
						NULL, 0,
						VolumeInfo, sizeof( NTFS_VOLUME_DATA_BUFFER ) );

	if( status != STATUS_SUCCESS ) {
		
		printf("\nError obtaining NTFS information: ");
		//PrintNtError( status );
		CloseHandle( volumeHandle );
		return FALSE;
	}

	//
	// Close the volume
	//
	CloseHandle( volumeHandle );

	return TRUE;
}


void PrintWin32Error( DWORD ErrorCode )
{
	LPVOID lpMsgBuf;
 
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					NULL, ErrorCode, 
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPTSTR) &lpMsgBuf, 0, NULL );
	printf("%s\n", lpMsgBuf );
	LocalFree( lpMsgBuf );
}



BOOLEAN LocateNTDLLCalls()
{

 	if( !((*(void**)&NtFsControlFile) = (void *) GetProcAddress( GetModuleHandle("ntdll.dll"),
			"NtFsControlFile" )) ) {

		return FALSE;
	}

	return TRUE;
}


int _tmain(int argc, _TCHAR* argv[])
{
	LocateNTDLLCalls();

	NTFS_VOLUME_DATA_BUFFER		volumeInfo;
	GetNTFSInfo(3,&volumeInfo);

	HANDLE hFile=CreateFile( "D:\\$MFT", GENERIC_READ, 
					FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 
					0, 0 );

	if (hFile!=INVALID_HANDLE_VALUE)
	{
        CloseHandle(hFile);
	}
	else
		PrintWin32Error(GetLastError());

	
	return 0;
}

