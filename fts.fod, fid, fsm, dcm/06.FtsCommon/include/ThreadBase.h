#pragma once

#include "afxwin.h"



class CThreadBase
{
	//// Static ////////////////////////////////////
public:
	static void ReqStopAll();
protected:
	static bool	s_bReqStop;
	static unsigned int threadEntryTemp( void* p_pArg );
	
	
	//// Constructor, Destructor ///////////////////
public:
	CThreadBase();
	virtual ~CThreadBase();
	

	//// Method /////////////////////////////////////
public:
	bool Run();
	bool IsRun();
	void ReqStop();
	bool IsReqStop();
	bool WaitStop(  int p_nTimeout );
	
	
	//// Implmentation - Overriden //////////////////
protected:
	virtual void onThreadEntry() = 0;

	//// Implmentation //////////////////////////////
protected:
	void init();
	
	//// Implementation - Variable //////////////////
protected:
	bool	m_bRun; 	// ���� ���࿩��
	bool	m_bReqStop;	// ������ ���� ��û ����
};




