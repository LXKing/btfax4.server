#include "StdAfx.h"
#include "DcmDataTable.h"

CDcmDataTable::CDcmDataTable()
{
}

CDcmDataTable::CDcmDataTable(char* p_tableName)
{
	m_dataTableName = p_tableName;
	m_registed = false;
}

CDcmDataTable::~CDcmDataTable(void)
{
	Clear();
}

void CDcmDataTable::Clear()
{
	m_dataTableName = "";	
	ClearColumns();
	ClearRows();
}

void CDcmDataTable::ClearRows()
{
	list<CDcmDataRow*>::iterator itor;
	for(itor=m_dataRows.begin(); itor != m_dataRows.end(); itor++)
		delete *itor;

	m_dataRows.clear();
}

void CDcmDataTable::ClearGarbageRows()
{
	list<CDcmDataRow*> removeItems;
	list<CDcmDataRow*>::iterator itor;
	for(itor=m_dataRows.begin(); itor != m_dataRows.end(); itor++)
	{	
		if((*itor)->m_doDelete)
			removeItems.push_back(*itor);
	}

	list<CDcmDataRow*>::iterator rItor;
	for(rItor=removeItems.begin(); rItor != removeItems.end(); rItor++)
	{	
		(*rItor)->Clear();
		m_dataRows.remove(*rItor);
		delete *rItor;
	}

	removeItems.clear();
}

void CDcmDataTable::ClearColumns()
{
	list<CDcmDataColumn*>::iterator itor;
	for(itor=m_dataColumns.begin(); itor != m_dataColumns.end(); itor++)
		delete *itor;

	m_dataColumns.clear();
}

bool CDcmDataTable::AddColumn(char* p_columnName, ColumnType columnType)
{
	if(ExistsColumn(p_columnName))
	{	
		SetErrMsg("ExistsColumn : %s", p_columnName);
		return false;
	}

	CDcmDataColumn *col = new CDcmDataColumn(p_columnName, columnType); 
	
	// zero base
	col->m_table = &*this;
	col->SetIndex(GetColumnCount());
	m_dataColumns.push_back(col);	
	return true;
}

CDcmDataRow* CDcmDataTable::NewRow()
{
	CDcmDataRow *row = new CDcmDataRow();
	row->m_table = &*this;
	return row;
}

void CDcmDataTable::AddRow(CDcmDataRow* p_row)
{
	p_row->SetRowIndex(GetRowCount());
	m_dataRows.push_back(p_row);
}

CDcmDataColumn* CDcmDataTable::GetColumn(char* p_columnName)
{
	CDcmDataColumn* pCol = NULL;
	list<CDcmDataColumn*>::iterator itor;	
	for(itor=m_dataColumns.begin(); itor != m_dataColumns.end(); itor++)
	{	
		if(strcmp((*itor)->GetColumnName().c_str(), p_columnName) == 0)
		{
			pCol = (*itor);
			break;
		}
	}
	return pCol;
}

CDcmDataColumn* CDcmDataTable::GetColumn(int p_ColumnIdx)
{
	CDcmDataColumn* pCol = NULL;
	list<CDcmDataColumn*>::iterator itor;
	for(itor=m_dataColumns.begin(); itor != m_dataColumns.end(); itor++)
	{	
		if((*itor)->GetIndex() == p_ColumnIdx)
		{
			pCol = *itor;
			break;
		}
	}

	return pCol;
}

int CDcmDataTable::GetColumnCount()
{
	return m_dataColumns.size();
}

int CDcmDataTable::GetRowCount()
{
	return m_dataRows.size();
}

int CDcmDataTable::GetRTDataRowCount()
{
	int cnt = 0;
	list<CDcmDataRow*>::iterator itor;
	for(itor=m_dataRows.begin(); itor != m_dataRows.end(); itor++)
	{
		if(!(*itor)->m_isBulkRow)
			cnt++;
	}

	return cnt;
}

int CDcmDataTable::GetBulkDataRowCount()
{
	int cnt = 0;
	list<CDcmDataRow*>::iterator itor;
	for(itor=m_dataRows.begin(); itor != m_dataRows.end(); itor++)
	{
		if((*itor)->m_isBulkRow)
			cnt++;
	}

	return cnt;
}

bool CDcmDataTable::ExistsColumn(const char* p_columnName)
{
	list<CDcmDataColumn*>::iterator itor;
	for(itor=m_dataColumns.begin(); itor != m_dataColumns.end(); itor++)
	{	
		if(!strcmp((*itor)->GetColumnName().c_str(), p_columnName))
			return true;
	}

	return false;
}

void CDcmDataTable::SetErrMsg(char* Format, ...)
{	
	char	Buff[100];	
	va_list	Args;
	memset(Buff, 0x00, sizeof(Buff));
	va_start( Args, Format );
	vsprintf_s( Buff, Format, Args );
	va_end( Args );

	m_errMsg = Buff;
}

char* CDcmDataTable::GetErrMsg()
{	
	char stream[100];
	strcpy(stream, m_errMsg.c_str());
	return stream;
}