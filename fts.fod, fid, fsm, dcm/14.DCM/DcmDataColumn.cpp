#include "StdAfx.h"
#include "DcmDataTable.h"
#include "DcmDataColumn.h"

CDcmDataColumn::CDcmDataColumn(void)
{
}

CDcmDataColumn::CDcmDataColumn(char* p_columnName, ColumnType p_columnType)
{
	Clear();
	m_columnName = p_columnName;
	m_columnType = p_columnType;
}

CDcmDataColumn::~CDcmDataColumn(void)
{
}

void CDcmDataColumn::SetIndex(int p_idx)
{
	m_columnIndex = p_idx;
}

int CDcmDataColumn::GetIndex()
{
	return m_columnIndex;
}

string CDcmDataColumn::GetColumnName()
{
	return m_columnName;
}


void CDcmDataColumn::Clear()
{
	m_columnIndex = -1;
	m_columnName = "";
	m_columnType = DcmColumnType_None;
}