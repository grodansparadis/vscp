#include <iostream.h>
extern "C"
{
void * test(char *);
}


/*-----------------------------------------------------------
  test with explicit load of a dll
  -----------------------------------------------------------*/
#include <windows.h>
#include <stdio.h>
#include "base.h"

//typedef int (WINAPI *DLLTEST)(int);

int main()
{
  parent *(*foo)(void)=0;
  static HINSTANCE  hLibrary = NULL;
  cerr << "before lib load" << endl;
  if ((hLibrary = LoadLibraryEx ("libchild1.dll", NULL,LOAD_WITH_ALTERED_SEARCH_PATH )) == NULL)
    {
      cout << "Unable to load library " << "libchild1.dll" << endl;
      return 0 ;
    }
  cerr << "before proc load\n" << endl;
  if ( (foo = (parent *(*)(void)) GetProcAddress (hLibrary,"CreateObject")) == NULL) 
    {
      cout << "Unable to find CreateObject" << endl;
      return 0 ;
    }
	cerr << "before proc run\n" << endl;

  parent *arse_y = (*foo)();
  arse_y->foobar();

  fflush(stdout);

  if (hLibrary)
    FreeLibrary (hLibrary) ;

}