/*-----------------------------------------------------------
  test with explicit load of a dll
  -----------------------------------------------------------*/
#include <windows.h>
#include <stdio.h>

typedef int (WINAPI *DLLTEST)(int);

void *test(char *dll)
{
  DLLTEST foo;
  static HINSTANCE  hLibrary = NULL;
  fprintf(stderr,"before lib load\n");
  if ((hLibrary = LoadLibraryEx (dll, NULL,LOAD_WITH_ALTERED_SEARCH_PATH )) == NULL)
    {
      printf("Unable to load library %s\r\n",dll);
      return 0 ;
    }
  fprintf(stderr,"before proc load\n");
  if ( (foo = (DLLTEST) GetProcAddress (hLibrary,"doit")) == NULL) 
    {
      printf("Unable to find doit \r\n");
      return 0 ;
    }
  fprintf(stderr,"before proc run\n");

  foo(10);
  return((void *)foo);
/*
  printf("doit(5) returns %d\n", foo(5));
  if (hLibrary)
    FreeLibrary (hLibrary) ;
*/
}

