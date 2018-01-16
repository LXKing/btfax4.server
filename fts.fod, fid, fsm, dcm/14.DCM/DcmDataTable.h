#pragma once

#include <iostream>
#include <list>
#include <string>
#include "DcmDataColumn.h"
#include "DcmDataRow.h"

// iCore..
#include "iLib.h"
#include "iSocket.h"
#include "iThread.h"
#include "iMutex.h"
#include "iType.h"
// iCore

using namespace std;

class CDcmDataTable
{
// constructor && destructor
public:
	CDcmDataTable();
	CDcmDataTable(char* p_tableName);
	~CDcmDataTable(void);

// methods
public:
	void Clear();
	void ClearRows();
	void ClearGarbageRows();
	void ClearColumns();
	bool AddColumn(char* p_columnName, ColumnType columnType);
	
	CDcmDataRow* NewRow();
	void AddRow(CDcmDataRow*);
	CDcmDataColumn* GetColumn(char* p_columnName);
	CDcmDataColumn* GetColumn(int p_ColumnIdx);
	int GetColumnCount();
	int GetRowCount();
	int GetRTDataRowCount();
	int GetBulkDataRowCount();
	bool ExistsColumn(const char* p_columnName);
	
	char* GetErrMsg();

	// fields ..
	string					m_errMsg;
	string					m_dataTableName;
	bool					m_registed;
	list<CDcmDataColumn*>	m_dataColumns;
	list<CDcmDataRow*>		m_dataRows;

	iMutex				m_mutexLock;
protected:
	void SetErrMsg(char* Format, ...);


protected:	
	
};

