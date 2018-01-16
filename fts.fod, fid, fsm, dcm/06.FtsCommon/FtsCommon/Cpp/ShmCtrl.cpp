#include "StdAfx.h"
#include "ShmCtrl.h"


CShmCtrl::CShmCtrl(void)
{
	Clear();
}


CShmCtrl::~CShmCtrl(void)
{
	Clear();
	ReleaseShm();
}

bool CShmCtrl::OpenOrCreateShm(const char* p_pKey)
{
	if(!OpenShm(p_pKey))
	{
		ReleaseShm();
		if(!CreateShm(p_pKey))
			return false;
	}

	return true;
}

bool CShmCtrl::CreateShm(const char* p_pKey)
{	
	// 공유메모리 생성
	m_hMapFile = CreateFileMapping( INVALID_HANDLE_VALUE,   
									NULL,                   
									PAGE_READWRITE,         
									0,                      
									sizeof(m_pShmDatas),					
									p_pKey 
									);
    
	if (m_hMapFile == NULL) 
    {	
		SetErrMsg("CreateShm()", "CreateFileMapping failed.");
        return false;
    }

	// 맵뷰
	m_pView = MapViewOfFile( m_hMapFile,             
							 FILE_MAP_ALL_ACCESS,    
							 0,                      
							 0,						
							 sizeof(m_pShmDatas) 
							 );

    if (m_pView == NULL)
    { 
		SetErrMsg("CreateShm()", "MapViewOfFile failed.");
        return false;
    }
    
	// 데이터 복사
	for(int i=0; i<MAX_SIZE; i++)
		m_pShmDatas[i].Clear();

	memcpy_s(m_pView, sizeof(m_pShmDatas), m_pShmDatas, sizeof(m_pShmDatas));

	m_strKey = p_pKey;
	
	return true;
}

bool CShmCtrl::OpenShm(const char* p_pKey)
{	
	if(m_hMapFile == NULL)
	{
		m_hMapFile = OpenFileMapping( FILE_MAP_ALL_ACCESS,          
									  FALSE,                  
									  p_pKey
									);
	}
	
	if (m_hMapFile == NULL) 
	{
		SetErrMsg("OpenShm()", "OpenFileMapping failed.");
		return false;
	}
    
	if(m_pView == NULL)
	{
		m_pView = MapViewOfFile( m_hMapFile,               
							 FILE_MAP_ALL_ACCESS,          
							 0,                      
							 0,            
							 sizeof(m_pShmDatas)
							 );
	}
	
	if (m_pView == NULL)
	{
		SetErrMsg("OpenShm()", "MapViewOfFile failed.");
		return false;
	}

	m_strKey = p_pKey;

	return true;
}

SHM_CH_MONI_DATA* CShmCtrl::GetShmData(int p_nIdx)
{
	if (m_hMapFile == NULL) 
	{
		SetErrMsg("GetShmData()", "Not Exists MappingFile.");
		return NULL;
	}

	if (m_pView == NULL)
	{
		SetErrMsg("GetShmData()", "Not Exists MapViewOfFile.");
		return NULL;
	}

	if(p_nIdx < 0)
	{
		SetErrMsg("GetShmData()", "Invalid Index.");
		return NULL;
	}

	SHM_CH_MONI_DATA *pShmData = (SHM_CH_MONI_DATA*)m_pView;
	//pShmData += p_nIdx;
	//return pShmData;
	return &pShmData[p_nIdx];
}
void CShmCtrl::GetShmData(int p_nIdx, SHM_CH_MONI_DATA* p_pShmData)
{
	if (m_hMapFile == NULL) 
	{
		SetErrMsg("GetShmData()", "Not Exists MappingFile.");
		return;
	}

	if (m_pView == NULL)
	{
		SetErrMsg("GetShmData()", "Not Exists MapViewOfFile.");
		return;
	}

	if(p_nIdx < 0)
	{
		SetErrMsg("GetShmData()", "Invalid Index.");
		return;
	}

	p_pShmData = &((SHM_CH_MONI_DATA*)m_pView)[p_nIdx];
}

void CShmCtrl::ReleaseShm()
{	
	if (m_hMapFile != NULL)
		UnmapViewOfFile(m_pView);
    
    if (m_pView != NULL)
		CloseHandle(m_hMapFile);
}

void CShmCtrl::SetErrMsg(const char* pFuncName, const char* pErrMsg)
{
	m_strErrMsg.Format("[%s] %s 0x%08lx", pFuncName, pErrMsg, GetLastError());
}

CString CShmCtrl::GetErrMsg()
{
	return m_strErrMsg;
}

void CShmCtrl::Clear()
{
	m_strKey	= "";
    m_pView		= NULL;
    m_hMapFile	= NULL;
}