#pragma once

#define INITIAL_DATA  -2156765124

#include <iostream>
#include <list>
#include <string>

#include "DcmDataTable.h"
#include "DcmDataColumn.h"
#include "DcmDataRow.h"

using namespace std;

class CDcmDataCell
{
public:
	CDcmDataCell(void);
	~CDcmDataCell(void);

	void SetCellId(long p_cellId);
	void SetValueStr(char* p_columnName, char* p_value);
	void SetValueInt(char* p_columnName, int p_value);
	void SetValueLong(char* p_columnName, long p_value);
	void SetValueDouble(char* p_columnName, double p_value);

	void GetValueStr(char* buf);
	int GetValueInt();
	long GetValueLong();
	double GetValueDouble();
	ColumnType	m_columnType;
	long 			m_cellId;	
protected:
	void Clear();


// fields
protected:
	
	string			m_columnName;
	string			m_valueStr;
	int				m_valueInt;
	long			m_valueLong;
	double			m_valueDouble;
};

