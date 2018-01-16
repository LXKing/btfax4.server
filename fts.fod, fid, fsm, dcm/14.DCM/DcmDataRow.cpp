#include "StdAfx.h"
#include "DcmDataTable.h"
#include "DcmDataColumn.h"
#include "DcmDataRow.h"
#include "DcmDataCell.h";

#include <list>

CDcmDataRow::CDcmDataRow(void)
{	
	m_doDelete = false;
}

CDcmDataRow::~CDcmDataRow(void)
{
}

void CDcmDataRow::SetRowIndex(int p_idx)
{
	m_rowIndex = p_idx;
}

void CDcmDataRow::Clear()
{
	list<CDcmDataCell*>::iterator itor;
	for(itor=m_dataCells.begin(); itor != m_dataCells.end(); itor++)
		delete *itor;

	m_dataCells.clear();
}

list<CDcmDataCell*> CDcmDataRow::GetDataCells()
{
	return m_dataCells;
}

void CDcmDataRow::SetValueStr(char* p_columnName, char* p_value)
{	
	CDcmDataColumn* col = m_table->GetColumn(p_columnName);
	if(col == NULL)
		return;

	CDcmDataCell *cell = new CDcmDataCell();
	cell->SetCellId(GetCellCount());
	cell->m_columnType = DcmColumnType_String;
	cell->SetValueStr(p_columnName, p_value);

	PushBackCell(cell);
	//m_dataCells.push_back(cell);
}

void CDcmDataRow::SetValueInt(char* p_columnName, int p_value)
{	
	CDcmDataColumn* col = m_table->GetColumn(p_columnName);
	if(col == NULL)
		return;

	CDcmDataCell *cell = new CDcmDataCell();
	cell->SetCellId(GetCellCount());
	cell->m_columnType = DcmColumnType_Integer;
	cell->SetValueInt(p_columnName, p_value);
	PushBackCell(cell);
	//m_dataCells.push_back(cell);
}

void CDcmDataRow::SetValueLong(char* p_columnName, long p_value)
{	
	CDcmDataColumn* col = m_table->GetColumn(p_columnName);
	if(col == NULL)
		return;

	CDcmDataCell *cell = new CDcmDataCell();
	cell->SetCellId(GetCellCount());
	cell->m_columnType = DcmColumnType_Long;
	cell->SetValueLong(p_columnName, p_value);
	PushBackCell(cell);
	//m_dataCells.push_back(cell);
}

void CDcmDataRow::PushBackCell(CDcmDataCell* pCell)
{	
	m_dataCells.push_back(pCell);
}

int CDcmDataRow::GetCellCount()
{	
	int size  = 0;
	size = m_dataCells.size();
	if(size <0)
		return 0;

	return size;
}