#include "base.h"

class child1: public parent
	{
	public:
	virtual void foobar(void);
	};

extern "C"
{
#include <windows.h> 
#include <stdio.h>

int WINAPI DllMain (HINSTANCE,DWORD,PVOID);

int doittoo(int);

int doit (int);

parent *CreateObject(void);
}