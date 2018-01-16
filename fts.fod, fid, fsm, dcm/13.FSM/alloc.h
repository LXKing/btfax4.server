#pragma once

typedef struct{
	unsigned	leg;
	int		c;
}App_Call_Handle;

int		AllocChan();
int		AllocAppCallHandle(unsigned leg);
void	ResetAppCallHandle(App_Call_Handle * h);

