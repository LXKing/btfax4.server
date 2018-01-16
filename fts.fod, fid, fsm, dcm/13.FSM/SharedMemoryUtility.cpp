#include "StdAfx.h"
#include "SharedMemoryUtility.h"

sip_share * shmem;
unsigned int	SHMID_FSM;

CSharedMemory::CSharedMemory()
{
}

CSharedMemory::~CSharedMemory()
{
}

// ---------------------------------------------------------------------------
// Module 명		: int MakeSharedMemory()
// ---------------------------------------------------------------------------
// Description	: Shared Memory 생성
// ---------------------------------------------------------------------------
// Argument		: void
// ---------------------------------------------------------------------------
// Return		: 0			Shared Memory 생성 SUCC
//				  1101		Shared Memory 생성 FAIL
//				  1102		Shared Memory 이미 존재
// ---------------------------------------------------------------------------
void* CSharedMemory::MakeSharedMemory( const char* p_szName, int p_nSize )
{
	SECURITY_DESCRIPTOR	sd={0};
	SECURITY_ATTRIBUTES sa={0};
	HANDLE	hMapFile;
	void*	pMemory;
    
	// ---------------------------------------------------------------------------
	// Object에 대한 Security Access를 생성
	// ---------------------------------------------------------------------------
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, 1, 0, 0);
	
	sa.nLength=sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle=0;
	sa.lpSecurityDescriptor=&sd;

	// ---------------------------------------------------------------------------
	// Shared Memory 생성
	// ---------------------------------------------------------------------------
	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE, 0, p_nSize, p_szName);
	if( hMapFile == 0 )
		return	NULL;

	if( GetLastError() == ERROR_ALREADY_EXISTS ) 
	{
		CloseHandle(hMapFile);
		return	NULL;
	}

	// ---------------------------------------------------------------------------
	// 생성한 Shared Memory를 내부 구조체에 Sttach 시킨다.
	// ---------------------------------------------------------------------------
	pMemory = MapViewOfFile( hMapFile, FILE_MAP_READ|FILE_MAP_WRITE, 0, 0, 0 );
	if( pMemory == NULL )
	{
		CloseHandle(hMapFile);
		return	NULL;
	}

	return pMemory;
}


// ---------------------------------------------------------------------------
// Module 명		: void AttachSharedMemory()
// ---------------------------------------------------------------------------
// Description	: Semaphore Open 및 Lock
//					(fasip_in.exe 프로세스가 중복하여 기동할 수 없게 함)
// ---------------------------------------------------------------------------
// Argument		: void
// ---------------------------------------------------------------------------
// Return		: 0			Signal 받은 상태
//				  exit		Signal 받지못한 상태
// ---------------------------------------------------------------------------
void* CSharedMemory::OpenSharedMemory( const char* p_szName, int p_nSize, bool p_bCreate )
{
	HANDLE	hMapFile;

	// ---------------------------------------------------------------------------
	// P1. Shared Memory Open (Write 권한)
	// ---------------------------------------------------------------------------
	hMapFile = OpenFileMapping( FILE_MAP_WRITE, 0, p_szName );
	if( hMapFile != NULL )
		return MapViewOfFile( hMapFile, FILE_MAP_WRITE, 0, 0, 0 );

	return MakeSharedMemory( p_szName, p_nSize );
}



CCdr::CCdr()
{
}

CCdr::~CCdr()
{
}


// ---------------------------------------------------------------------------
// Module 명		: void InitCdr()
// ---------------------------------------------------------------------------
// Description	: CDR File Open
// ---------------------------------------------------------------------------
// Argument		: void
// ---------------------------------------------------------------------------
// Return		: void
// ---------------------------------------------------------------------------
FILE	* cdrfp;
void CCdr::InitCdr()
{
	SYSTEMTIME	st;
	char	logfile[256];

	GetLocalTime(&st);
	sprintf(logfile, "%s\\incdr\\%4d%02d%02d.cdr", (LPCSTR)CConfig::EXEC_PATH, st.wYear, st.wMonth, st.wDay);
	cdrfp=fopen(logfile, "at");
}


// ---------------------------------------------------------------------------
// Module 명		: void WriteCdr()
// ---------------------------------------------------------------------------
// Description	: 특정 CH의 CDR File Write
// ---------------------------------------------------------------------------
// Argument		: int i;		0 based CH 번호
// ---------------------------------------------------------------------------
// Return		: void
// ---------------------------------------------------------------------------
void CCdr::WriteCdr(int i)
{
	SYSTEMTIME	st;
	struct tm	b;
	dev_state * d;
	fax_rx_start * s;									// 팩스 수신정보 (발신,착신,Remote IP/Port 등)
	char	direction, disconnector;

	GetLocalTime(&st);

	d=shmem->d+i;
	s=&d->s;

	direction = d->call_direction;

	if(d->disconnector)									// 누가 끊었는지
		disconnector='L';
	else
		disconnector='R';

	b=*localtime(&d->start_time);						// 호시작시간

	fprintf(cdrfp, "%c, %3d, %c, %02d%02d:%02d%02d%02d, %02d%02d:%02d%02d%02d, %s, %s\n", direction, i, disconnector, b.tm_mon+1, b.tm_mday, b.tm_hour, b.tm_min, b.tm_sec, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, s->from, s->to);
}


