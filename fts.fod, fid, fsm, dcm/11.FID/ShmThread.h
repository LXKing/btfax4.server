#pragma once

#include "ThreadBase.h"
#include <list>
using namespace std;


class CShmThread : public CThreadBase
{
public:
	static CShmThread* Inst();
protected:
	static CShmThread* s_pInstance;

public:
	CShmThread(void);
	virtual ~CShmThread(void);
	bool	IsChangedDate();

// ovveride
private:
	virtual void onThreadEntry();


private:
	struct	tm* m_currTime;

};

