#include "StdAfx.h"
#include "DcmDataSet.h"

CDcmDataSet::CDcmDataSet(char* p_dsName)
{
	m_dataSetName = p_dsName;
}


CDcmDataSet::CDcmDataSet(void)
{
	m_dataSetName = "DATASET";
}


CDcmDataSet::~CDcmDataSet(void)
{
}

void CDcmDataSet::AddTable(CDcmDataTable* p_table)
{
	m_dataTables.push_back(p_table);
}

CDcmDataTable* CDcmDataSet::GetNotRegistedTable()
{
	list<CDcmDataTable*>::iterator itor;
	for(itor=m_dataTables.begin(); itor != m_dataTables.end(); itor++)
	{
		if(!(*itor)->m_registed)
			return (*itor);
	}

	return NULL;
}

void CDcmDataSet::UnRegistedTable()
{
	int cnt = 0;
	list<CDcmDataTable*>::iterator itor;
	for(itor=m_dataTables.begin(); itor != m_dataTables.end(); itor++)
		(*itor)->m_registed = false;
}

int CDcmDataSet::GetRegistedTableCount()
{
	int cnt = 0;
	list<CDcmDataTable*>::iterator itor;
	for(itor=m_dataTables.begin(); itor != m_dataTables.end(); itor++)
	{
		if(!(*itor)->m_registed)
			cnt++;
	}

	return cnt;
}

void CDcmDataSet::Clear()
{
	/*list<CDcmDataTable*>::iterator itor;
	for(itor=m_dataTables.begin(); itor != m_dataTables.end(); itor++)
		delete *itor;*/

	m_dataTables.clear();
}