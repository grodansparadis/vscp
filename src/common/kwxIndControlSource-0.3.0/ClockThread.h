#ifndef __CLOCKTHREAD_H__
#define __CLOCKTHREAD_H__

#include <wx\thread.h>

class CClockThread : public wxThread
{
public:
	CClockThread();
	virtual ~CClockThread();

	void *Entry( void );

};

#endif // __CLOCKTHREAD_H__
