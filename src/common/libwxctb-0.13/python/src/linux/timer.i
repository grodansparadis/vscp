%{
#include "wx/ctb-0.13/linux/timer.h"
%}

%include cpointer.i

// lets create new fuctions for pointer handling in python (for int *exitflag)
%pointer_functions(int, intp);

// perhaps we doesn''t need timer_control to export
// but we need if we want to inherit from timer in python
struct timer_control
{
    unsigned int usecs;
    int *exitflag;
    void* (*exitfnc)(void*);
};

class timer
{
protected:
    timer_control control;
    int stopped;
    pthread_t tid;
    unsigned int timer_secs;
public:
    timer(unsigned int msec,int* exitflag,void*(*exitfnc)(void*)=NULL);
    ~timer();
    int start();
    int stop();
};

void sleepms(unsigned int ms);
