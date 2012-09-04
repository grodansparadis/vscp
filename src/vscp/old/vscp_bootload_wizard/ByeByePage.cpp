///////////////////////////////////////////////////////////////////////////////
// ByeByePage.cpp: implementation of the CByeByePage class.
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
// $RCSfile: ByeByePage.cpp,v $                                       
// $Date: 2005/01/05 12:50:58 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 
///////////////////////////////////////////////////////////////////////////////

#include "ByeByePage.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

wxGoodbyPage::wxGoodbyPage( wxWizard *parent )
								: wxWizardPageSimple( parent )
{
       
	(void)new wxStaticText( this, 
					-1, 
					_T("The bootloading pocess has finished.\n")
					_T("\n")
					_T("The bootloader has restarted your device(s) into application mode.\n")
					_T("You now need to test your device(s) and make sure it/they work as \n")
					_T("expected.\n\n")
					_T("Thank you  for using this free software.\n")
					_T("Please support the VSCP/CANAL mother project at http://www.vscp.org.\n")
				);
  
}
