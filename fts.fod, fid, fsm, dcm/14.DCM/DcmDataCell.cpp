#include "StdAfx.h"
#include "DcmDataTable.h"
#include "DcmDataColumn.h"
#include "DcmDataRow.h"
#include "DcmDataCell.h"
#include "IConvEncoder.h"

CDcmDataCell::CDcmDataCell(void)
{
	Clear();
}

CDcmDataCell::~CDcmDataCell(void)
{
	Clear();
}

void CDcmDataCell::Clear()
{
	m_cellId	= -1;
	m_valueStr	= "";
	m_valueInt	= INITIAL_DATA;
	m_valueLong = INITIAL_DATA;
	m_valueDouble = INITIAL_DATA;
}

void CDcmDataCell::SetCellId(long p_cellId)
{
	m_cellId = p_cellId;
}

void CDcmDataCell::SetValueStr(char* p_columnName, char* p_value)
{
	m_columnName	= p_columnName;
	m_valueStr		= p_value;
}

void CDcmDataCell::SetValueInt(char* p_columnName, int p_value)
{	
	m_columnName	= p_columnName;
	m_valueInt		= p_value;
}

void CDcmDataCell::SetValueLong(char* p_columnName, long p_value)
{	
	m_columnName	= p_columnName;
	m_valueLong		= p_value;
}

void CDcmDataCell::SetValueDouble(char* p_columnName, double p_value)
{
	m_columnName	= p_columnName;
	m_valueDouble	= p_value;
}

void CDcmDataCell::GetValueStr(char* buf)
{	
	strcpy(buf, m_valueStr.c_str());
	CIConvEncoder::ConvertToUtf8(buf, sizeof(buf));
}

int CDcmDataCell::GetValueInt()
{	
	return m_valueInt;
}

long CDcmDataCell::GetValueLong()
{	
	return m_valueLong;
}

double CDcmDataCell::GetValueDouble()
{
	return m_valueDouble;
}