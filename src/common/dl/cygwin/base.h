#include <iostream.h>
#include <stdlib.h>
#include <stdio.h>

class parent
{
public:
	virtual void foobar(void) {}; 
};

extern "C"
{
parent *CreateObject(void);
}

