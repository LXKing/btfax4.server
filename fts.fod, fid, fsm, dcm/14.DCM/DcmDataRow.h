#pragma once
#include <iostream>
#include <list>
#include <string>
#include "DcmDataTable.h"
#include "DcmDataCell.h"

// iCore..
#include "iLib.h"
#include "iSocket.h"
#include "iThread.h"
#include "iMutex.h"
#include "iType.h"
// iCore

using namespace std;

class CDcmDataTable;

class CDcmDataRow
{
public:
	CDcmDataRow(void);
	~CDcmDataRow(void);

	void Clear();
	void SetRowIndex(int p_idx);
	void SetValueStr(char* p_columnName, char* value);
	void SetValueInt(char* p_columnName, int value);
	void SetValueLong(char* p_columnName, long value);
	int GetCellCount();

	void PushBackCell(CDcmDataCell* pCell);
	list<CDcmDataCell*> GetDataCells();

// fields
public :
	CDcmDataTable		*m_table;
	list<CDcmDataCell*>	m_dataCells;
	bool				m_doDelete;
	bool				m_isBulkRow;
protected:
	int					m_rowIndex;
	iMutex				m_mutexLock;
};


