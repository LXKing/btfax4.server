#include "StdAfx.h"
#include "EncryptApi.h"

// constructor && destructor
EncryptApi::EncryptApi(void)
{
    Clear();
}


EncryptApi::~EncryptApi(void)
{
    Clear();
}


// singleton..
EncryptApi* EncryptApi::s_pInstance = NULL;

EncryptApi* EncryptApi::Inst()
{
	if( s_pInstance == NULL )
		s_pInstance = new EncryptApi;

	return s_pInstance;
}

void EncryptApi::Clear()
{
    m_hMod = NULL;
    m_strDllPath = "";
}

bool EncryptApi::InitApi(CString strDllPath)
{
    m_hMod = LoadLibrary(strDllPath);
    if(m_hMod == NULL)
        return false;

    Init func = (Init)GetProcAddress(m_hMod, "Init");
    if(func == NULL)
        return false;

    int nRet = func();
    if(nRet <= 0)
        return false;
    
    m_strDllPath = strDllPath;
    
    return true;
}


bool EncryptApi::DisposeApi()
{
    if(m_hMod == NULL)
        return false;

    Dispose func = (Dispose)GetProcAddress(m_hMod, "Dispose");
    if(func == NULL)
        return false;

    int nRet = func();
    if(nRet <= 0)
        return false;
    
    return true;
}

bool EncryptApi::TryEncrypt(CString strSrc, CString &strOut)
{   
    if(m_hMod == NULL)
    {
        strOut = strSrc;
        return false;
    }
  
    Encrypt func = (Encrypt)GetProcAddress(m_hMod, "Encrypt");
    if(func == NULL)
        return false;
    
    CString strVal = func(strSrc);
    strOut = strVal;
    
    return true;
}

bool EncryptApi::TryDecrypt(CString strSrc, CString &strOut)
{
    if(m_hMod == NULL)
    {
        strOut = strSrc;
        return false;
    }
  
    Decrypt func = (Decrypt)GetProcAddress(m_hMod, "Decrypt");
    if(func == NULL)
        return false;
    
    CString strVal = func(strSrc);
    strOut = strVal;
    
    return true;
}

