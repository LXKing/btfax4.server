#pragma once

#include <stdio.h>
#include <Windows.h>
#include <iostream>

using namespace std;

// extern functions
typedef int                 (*Init)();
typedef int                 (*Dispose)();
typedef char*               (*Encrypt)(const char* szSrc);
typedef char*               (*Decrypt)(const char* szSrc);


class EncryptApi
{
public:
    EncryptApi(void);
    ~EncryptApi(void);

public:
	static EncryptApi* Inst();
protected:
	static EncryptApi* s_pInstance;


public:
    bool InitApi(CString strDllPath);
    bool DisposeApi();
    bool TryEncrypt(CString strSrc, CString &strOut);
    bool TryDecrypt(CString strSrc, CString &strOut);

protected:
    void Clear();

// fields
protected:
    HMODULE m_hMod;
    CString m_strDllPath;

};

