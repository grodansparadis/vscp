#include "stdio.h"
#include "child1.h"

void child1::foobar(void)
	{
	//cout << "Hello from within dll" << endl;
	printf("This is a test\n");
	}

parent *CreateObject(void)
	{
	child1 *arse = new child1;
	return((parent *)arse);
	}
