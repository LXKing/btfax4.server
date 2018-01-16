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
// Module ��		: int MakeSharedMemory()
// ---------------------------------------------------------------------------
// Description	: Shared Memory ����
// ---------------------------------------------------------------------------
// Argument		: void
// ---------------------------------------------------------------------------
// Return		: 0			Shared Memory ���� SUCC
//				  1101		Shared Memory ���� FAIL
//				  1102		Shared Memory �̹� ����
// ---------------------------------------------------------------------------
void* CSharedMemory::MakeSharedMemory( const char* p_szName, int p_nSize )
{
	SECURITY_DESCRIPTOR	sd={0};
	SECURITY_ATTRIBUTES sa={0};
	HANDLE	hMapFile;
	void*	pMemory;
    
	// ---------------------------------------------------------------------------
	// Object�� ���� Security Access�� ����
	// ---------------------------------------------------------------------------
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, 1, 0, 0);
	
	sa.nLength=sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle=0;
	sa.lpSecurityDescriptor=&sd;

	// ---------------------------------------------------------------------------
	// Shared Memory ����
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
	// ������ Shared Memory�� ���� ����ü�� Sttach ��Ų��.
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
// Module ��		: void AttachSharedMemory()
// ---------------------------------------------------------------------------
// Description	: Semaphore Open �� Lock
//					(fasip_in.exe ���μ����� �ߺ��Ͽ� �⵿�� �� ���� ��)
// ---------------------------------------------------------------------------
// Argument		: void
// ---------------------------------------------------------------------------
// Return		: 0			Signal ���� ����
//				  exit		Signal �������� ����
// ---------------------------------------------------------------------------
void* CSharedMemory::OpenSharedMemory( const char* p_szName, int p_nSize, bool p_bCreate )
{
	HANDLE	hMapFile;

	// ---------------------------------------------------------------------------
	// P1. Shared Memory Open (Write ����)
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
// Module ��		: void InitCdr()
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
// Module ��		: void WriteCdr()
// ---------------------------------------------------------------------------
// Description	: Ư�� CH�� CDR File Write
// ---------------------------------------------------------------------------
// Argument		: int i;		0 based CH ��ȣ
// ---------------------------------------------------------------------------
// Return		: void
// ---------------------------------------------------------------------------
void CCdr::WriteCdr(int i)
{
	SYSTEMTIME	st;
	struct tm	b;
	dev_state * d;
	fax_rx_start * s;									// �ѽ� �������� (�߽�,����,Remote IP/Port ��)
	char	direction, disconnector;

	GetLocalTime(&st);

	d=shmem->d+i;
	s=&d->s;

	direction = d->call_direction;

	if(d->disconnector)									// ���� ��������
		disconnector='L';
	else
		disconnector='R';

	b=*localtime(&d->start_time);						// ȣ���۽ð�

	fprintf(cdrfp, "%c, %3d, %c, %02d%02d:%02d%02d%02d, %02d%02d:%02d%02d%02d, %s, %s\n", direction, i, disconnector, b.tm_mon+1, b.tm_mday, b.tm_hour, b.tm_min, b.tm_sec, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, s->from, s->to);
}


