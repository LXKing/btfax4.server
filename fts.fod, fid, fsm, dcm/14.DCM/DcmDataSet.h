#pragma once

#include <iostream>
#include <list>
#include <string>
#include "DcmDataTable.h"

class CDcmDataSet
{
public:
	CDcmDataSet(char* p_dsName);
	CDcmDataSet(void);
	~CDcmDataSet(void);

	void AddTable(CDcmDataTable* p_table);
	void Clear();
	void UnRegistedTable();
	CDcmDataTable* GetNotRegistedTable();
	int GetRegistedTableCount();
	list<CDcmDataTable*>	m_dataTables;
	string m_dataSetName;
// fields
protected:
};

