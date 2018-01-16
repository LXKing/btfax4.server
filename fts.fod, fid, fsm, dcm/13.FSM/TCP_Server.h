#pragma once


#pragma once


#define MAXREG		32

enum{
	Wait_Sec=1,
};

int		TcpServerInit(int localport);
void	CheckAlive();
int		FaxAccept();
int		FaxRead(int i);
void	FaxDisconnected( int i, int cause );
void	FaxDisconnect( int i );
void	DecodeFaxMsg(int i);
void	FaxWrite(int i, int msgid, int chan, int len, char * para);
int		SetDevRegistered( fax_register * r, int id );
void	ResetDevRegistered( int id, fax_register * r );
void	LowerMaxFd(int fd);
void	GetMaxFd(int fd);

void	WriteSip(int msgid, int chan, int len, char * para);

unsigned __stdcall ProcessAppEvent(LPVOID arg);
int		RxIpcMsg();
void	IpcWrite(int msgid, int b, int chan, int len, char * para);


extern unsigned int	mili, last;
extern int		maxfd;
extern fd_set	readfds, allset;

extern int		sock_ipc;
extern int		sock_agent;