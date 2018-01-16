#ifndef __DB_CONNECTOR_H__
#define __DB_CONNECTOR_H__

#pragma once
// -------------------------------------------------------------------
// System Header
// -------------------------------------------------------------------
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

#include "DbConnection.h"

class CDbConnector
{
public:
	CDbConnector( CDbConnection* p_pConnection );
	virtual ~CDbConnector();

private:
	CDbConnection* m_pConnection;
};

#endif