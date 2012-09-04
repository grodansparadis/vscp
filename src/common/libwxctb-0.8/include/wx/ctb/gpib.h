/////////////////////////////////////////////////////////////////////////////
// Name:        gpib.h
// Purpose:     simple wrapper file
// Author:      Joachim Buermann
// Id:          $Id: gpib.h,v 1.1.1.1 2004/11/24 10:30:11 jb Exp $
// Copyright:   (c) 2004 Joachim Buermann
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __GPIB_BASE_H
#define __GPIB_BASE_H

#if defined (WIN32)
# include "wx/ctb/win32/gpib.h"
#else
# include "wx/ctb/linux/gpib.h"
#endif

#endif
// __GPIB_BASE_H
