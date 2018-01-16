#pragma once

#include <iostream>
#include <list>
#include <string>
#include "DcmDataTable.h"

// iCore..
#include "iLib.h"
#include "iSocket.h"
#include "iThread.h"
#include "iMutex.h"
#include "iType.h"
// iCore

typedef enum ColumnType
{
	DcmColumnType_None		= 0,
	DcmColumnType_Integer	= 1,
	DcmColumnType_String	= 2,
	DcmColumnType_Long		= 3,
	DcmColumnType_Double	= 4
};

using namespace std;

class CDcmDataTable;

class CDcmDataColumn
{
public:
	CDcmDataColumn(void);
	CDcmDataColumn(char* columnName, ColumnType columnType);
	~CDcmDataColumn(void);

// methods
public:
	void SetIndex(int p_idx);
	int GetIndex();
	string GetColumnName();

protected:
	void Clear();

// fields
public:
	CDcmDataTable	*m_table;
	int			m_columnIndex;
	string		m_columnName;
	ColumnType	m_columnType;

protected:
	iMutex				m_mutexLock;
};
