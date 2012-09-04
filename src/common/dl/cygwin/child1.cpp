#include <iostream.h>

#include "child1.h"

void child1::foobar(void)
	{
	printf("child1 here\n");
//	cout << "child1 here" << endl;
//i cant user cout here coz it crashes everytime, anyone got any ideas
//i have 2 suspicions, 1 ive just done this all wrong 2 that theres some reentrant problem
//C.
	}


parent *CreateObject(void)
	{
	child1 *arse = new child1;
	return((parent *)arse);
	}

int main(void)
{
}

int WINAPI DllMain(HINSTANCE hInstance , 
		           DWORD reason,
		           PVOID pvReserved)
{
  switch (reason) 
    {
    case DLL_PROCESS_ATTACH:
      break;
    case DLL_PROCESS_DETACH:
      break;
    case DLL_THREAD_ATTACH:
      break;
    case DLL_THREAD_DETACH:
      break;
    }
  return 1;
}

