#include "StdAfx.h"
#include "DbConnector.h"


CDbConnector::CDbConnector( CDbConnection* p_pConnection )
: m_pConnection( p_pConnection )
{
	m_pConnection->Open();
}

CDbConnector::~CDbConnector(void)
{
	m_pConnection->Close();
}

