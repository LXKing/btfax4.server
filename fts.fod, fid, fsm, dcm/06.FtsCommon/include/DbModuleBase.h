#ifndef __DB_MODULE_BASE_H__
#define __DB_MODULE_BASE_H__

#pragma once

#include <afxmt.h>
#include "DbConnection.h"
#include "DbConnector.h"
#include "ConfigBase.h"



// Macro : Get Recordset's field value
#define R_GET_STR(_recordset_, _variant_temp_, _field_name_, _value_var_) \
	_variant_temp_.SetString(_field_name_); \
	_variant_temp_ = _recordset_->Fields->Item[_variant_temp_]->Value; \
	V_STR_TO_STR(_variant_temp_, _value_var_);

#define R_GET_NUM(_recordset_, _variant_temp_, _field_name_, _value_var_) \
	_variant_temp_.SetString(_field_name_); \
	_variant_temp_ = _recordset_->Fields->Item[_variant_temp_]->Value; \
	V_NUM_TO_NUM(_variant_temp_, _value_var_);

#define R_GET_DBL(_recordset_, _variant_temp_, _field_name_, _value_var_) \
	_variant_temp_.SetString(_field_name_); \
	_value_var_ = _recordset_->Fields->Item[_variant_temp_]->Value; \
	
	//V_DBL_TO_DBL(_variant_temp_, _value_var_);

#define R_GET_DEC(_recordset_, _variant_temp_, _field_name_, _value_var_) \
	_variant_temp_.SetString(_field_name_); \
	_variant_temp_ = _recordset_->Fields->Item[_variant_temp_]->Value; \
	V_DEC_TO_STR(_variant_temp_, _value_var_);

#define R_GET_DAT(_recordset_, _variant_temp_, _field_name_, _value_var_) \
	_variant_temp_.SetString(_field_name_); \
	_variant_temp_ = _recordset_->Fields->Item[_variant_temp_]->Value; \
	V_DAT_TO_STR(_variant_temp_, _value_var_);

// Macro : Varaint to variable
#define V_STR_TO_STR(_variant_, _value_var_) \
	if( _variant_.vt != VT_NULL ) \
		_value_var_ = _variant_.bstrVal; \
	else \
		_value_var_ = "";

#define V_NUM_TO_NUM(_variant_, _value_var_) \
	if( _variant_.vt != VT_NULL ) \
		_value_var_ = _variant_.intVal; \
	else \
		_value_var_ = 0;

#define V_DBL_TO_DBL(_variant_, _value_var_) \
	if( _variant_.vt != VT_NULL ) \
		_value_var_ = _variant_.dblVal; \
	else \
		_value_var_ = 0;

#define V_DEC_TO_STR(_variant_, _value_var_) \
	if( _variant_.vt != VT_NULL ) \
		(_value_var_).Format("%s%I64u", ((_variant_.decVal.sign & DECIMAL_NEG) == DECIMAL_NEG) ? _T("-") : _T("") , \
									  _variant_.decVal.Lo64); \
	else \
		_value_var_ = "0";
#define V_DAT_TO_STR(_variant_, _value_var_) \
	if( _variant_.vt != VT_NULL ) \
		_value_var_ = COleDateTime(_variant_).Format("%Y/%m/%d %H:%M:%S"); \
	else \
		_value_var_ = "";

#define STR_TO_DEC(_value_var_, _decimal_) \
{ \
	const char* _value = (LPCSTR)_value_var_; \
	DECIMAL_SETZERO(_decimal_); \
	if( _value != NULL && _value[0] != '\0' ) \
	{ \
		if( _value[0] == '-' ) { \
			_decimal_.sign = DECIMAL_NEG; \
			_decimal_.Lo64 = _atoi64(_value+1); \
		} \
		else \
			_decimal_.Lo64 = _atoi64(_value); \
	} \
}

#define STR_TO_V_DEC(_value_var_, _variant_) \
	_variant_.ChangeType(VT_DECIMAL); \
	STR_TO_DEC(_value_var_, _variant_.decVal);


class CDbModuleBase
{
public:
	static CDbModuleBase* InstBase();
protected:
	static CDbModuleBase* s_pInstanceBase;

public:
	CDbModuleBase(void);
	virtual ~CDbModuleBase(void);

	int				ReadConfig(const char* pKey, CString* pValue);
	int				ReadConfig(const char* pKey, int* pValue);
	int				ReadConfig(const char* pKey, bool* pValue);
	void			SetErrorMessage(const char* pFuncName, _com_error &e);

public:
	CString			m_ErrorMessage;

protected:
	//bool			m_bConnected;						// Oracle 立加 咯何 (true/false)
	//_ConnectionPtr	m_pConn;	

	//CString			m_ConnStr;							// Oracle 立加 String
	CString			m_ID;								// Oracle 立加 ID
	CString			m_PWD;								// Oracle 立加 Password
};

#endif
