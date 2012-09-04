#include <stdio.h>
#include "child1.h"
#include <stdlib.h>
#include <dlfcn.h>


int main(void)
{
  void* handle = 0;
  parent *(*helloWorld)(void) = 0;
  const char* error = 0;
  
  handle = dlopen("./child1.so", RTLD_LAZY);
  if ( 0 == handle ) {
    error = dlerror();
    if (0 != error) {
      fprintf(stderr, "arse\n");
    }
    
    exit (EXIT_FAILURE);
  }
  
  helloWorld =  (parent *(*)(void))dlsym(handle, "CreateObject");
  if ( 0 == helloWorld ) {
    error = dlerror();
    if ( 0 != error ) {
      fprintf(stderr, "arse2");
      exit (EXIT_FAILURE);
    }
    
    /* else not an error */
  }

  parent *arse_y = (*helloWorld)();
  arse_y->foobar();
  
  dlclose(handle);
  
  return (EXIT_SUCCESS);
}
