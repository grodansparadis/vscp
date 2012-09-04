//#include <iostream.h>
#include <stdlib.h>

class parent
{
public:
	virtual void foobar(void) = 0; 
};

extern "C"
{
parent *CreateObject(void);
}

