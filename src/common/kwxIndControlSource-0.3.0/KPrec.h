/////////////////////////////////////////////////////////////////////////////
// Name:        KPrec.h
// Purpose:     Smart Precompiled Header Management
// Author:      Marco Cavallini
// Modified by: 
// Created:     
// RCS-ID:      
// Copyright:   (C)2004 KOAN Software - Bergamo - Italy
// Licence:     wxWindows
/////////////////////////////////////////////////////////////////////////////

#ifndef _KPREC_H_
#define _KPREC_H_

#include "wx/wxprec.h"


/*
Including file and linenumber in the debug memory allocation (memory leak dump)
http://www.litwindow.com/Knowhow/wxHowto/wxhowto.html

Include the following statements in your standard, precompiled headerfile
*/

#ifdef _DEBUG
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
#else
#define DEBUG_NEW new
#endif

/*
In every CPP file, include these statements immediately after the last 
#include statement (before the first malloc/new statement)
*/

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



#endif // _KPREC_H_
