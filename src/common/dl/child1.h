#include "base.h"

class child1 : public parent
{

public:

virtual void foobar(void);

};


extern "C"
{
parent *CreateObject(void);
}
