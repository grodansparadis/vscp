/////////////////////////////////////////////////////////////////////////////
// Name:        wxctb.h
// Purpose:
// Author:      Joachim Buermann
// Id:          $Id: wxctb.h,v 1.1.1.1 2004/11/24 10:30:11 jb Exp $
// Copyright:   (c) 2001-2003 Joachim Buermann
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __WXCTB_H
#define __WXCTB_H

#include "wx/ctb/expect.h"
#include "wx/ctb/getopt.h"
#include "wx/ctb/gpib.h"
#include "wx/ctb/serport.h"
#include "wx/ctb/timer.h"

/*
  IMPORTANT! This is only for wxMSW (wxWindow MSW port) users!
  Some of the included files above include <windows.h> on her part.
  <windows.h> defines a lot of identifiers which are also used in
  wxWin which gives you some urgly errors.
  Read the file wx/msw/winundef.h for more information.
*/

#if defined (__WXMSW__)
# include "wx/msw/winundef.h"
#endif

#endif
