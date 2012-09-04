///////////////////////////////////////////////////////////////////////////////
// Welcome.cpp: implementation of the CWelcome class.
//
// This file is part is part of CANAL (CAN Abstraction Layer)
// http://www.vscp.org)
//
// Copyright (C) 2000-2010 Ake Hedman, eurosource, <akhe@eurosource.se>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// $RCSfile: Welcome.cpp,v $                                       
// $Date: 2005/01/05 12:50:58 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 
///////////////////////////////////////////////////////////////////////////////

#include "Welcome.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


wxWelcomePage::wxWelcomePage( wxWizard *parent )
								: wxWizardPageSimple( parent )
{
       
	(void)new wxStaticText( this, 
					-1, 
					_T("This wizard will help you to load/replace the firmware on a VSCP node.\n")
					_T("\n")
					_T("You need to be hooked up to the network and have the new code\n")
					_T("available on your computer.\n\n")
					_T("Warning!!!.\n")
					_T("The bootloader process is always a dangerous one and may leave\n")
					_T("the node(s) you want to update in a 'broken' or non-functional state.\n")
					_T("You shold therefore know what you are doing!")
				);
  
}