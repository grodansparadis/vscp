/////////////////////////////////////////////////////////////////////////////
// Name:        NewWidget.cpp
// Purpose:     
// Author:      
// Modified by: 
// Created:     
// RCS-ID:      
// Copyright:   (C)2003 
// Licence:     wxWindows
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include <wx/image.h>
#include <wx/log.h>

#include "kwxNewWidget.h"
#include <wx/event.h>



//IMPLEMENT_DYNAMIC_CLASS(kwxNewWidget, wxWindow)

BEGIN_EVENT_TABLE(kwxNewWidget,wxWindow)
  EVT_PAINT(kwxNewWidget::OnPaint)
  EVT_MOUSE_EVENTS(kwxNewWidget::OnMouse)
  //EVT_MOTION(kwxNewWidget::OnMouse) 
END_EVENT_TABLE()

kwxNewWidget::kwxNewWidget(wxWindow* parent,
		                   const wxWindowID id,
		                   const wxString& label,
                           wxBitmap& defaultBitmap,
							wxBitmap& MoveMouseBitmap,
							wxBitmap& PressMouseBitmap,
                           const wxPoint& pos,
                           const wxSize& size,
		                   const long int style)
	: wxWindow(parent, id, pos, size, 0)
{

  if (parent)
    SetBackgroundColour(parent->GetBackgroundColour());
  else
    SetBackgroundColour(*wxLIGHT_GREY);

	mDefaultBitmap  = &defaultBitmap;
	mMoveMouseBitmap = &MoveMouseBitmap;
	mPressMouseBitmap = &PressMouseBitmap;


    int total_width = 0, total_height = 0 ;

	// se c'e' un bitmap ne uso le dimensioni
    if (mDefaultBitmap)
    {
      total_width  = mDefaultBitmap->GetWidth();  // NB! Should use largest bm
      total_height = mDefaultBitmap->GetHeight(); // NB! Should use largest bm
    }
	SetSize(total_width, total_height);


    //SetSize(size);
    SetAutoLayout(TRUE);
	Refresh();
	m_stato=0;
	m_oldstato=0;

}

void kwxNewWidget::SetLabel(wxString label)
{
	mLabelStr = label;
} 

void kwxNewWidget::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);
	dc.SetBackground(*wxTheBrushList->FindOrCreateBrush(GetBackgroundColour(),wxSOLID));
	dc.Clear();


	int w,h;
	
	GetClientSize(&w,&h);


	// se impostato n bitmap lo disegno
    //if (mDefaultBitmap)
	if(m_stato==0)
	{
		dc.DrawBitmap(*mDefaultBitmap, 0, 0, TRUE);
	}
	else if(m_stato==1)
	{
		dc.DrawBitmap(*mMoveMouseBitmap, 0, 0, TRUE);
	}
	else if(m_stato==2)
	{
		dc.DrawBitmap(*mPressMouseBitmap, 0, 0, TRUE);
	}


	// Cornice intorno
	dc.SetPen(*wxThePenList->FindOrCreatePen(*wxRED, 1, wxSOLID));
	dc.DrawLine(0,0,0,h-1);
	dc.DrawLine(0,0,w,0);
	dc.DrawLine(0,h-1,w,h-1);
	dc.DrawLine(w-1,0,w-1,h-1);
 
}

void kwxNewWidget::OnMouse(wxMouseEvent& event)
{
	if(MouseIsOnButton())
	{
		if (event.LeftIsDown())
		{
			m_stato=2;
		}
		else
		{
			m_stato=1;	
		}
	}
	if(event.Leaving())
	{
		m_stato=0;	
	}

	if(m_oldstato!=m_stato)
	{
		Refresh();
	}

	m_oldstato=m_stato;
	event.Skip();
}


bool kwxNewWidget::MouseIsOnButton()
{
  int cx=0,cy=0;
  int wbit,hbit;
  ClientToScreen(&cx,&cy);

  int cw=0,ch=0;
  this->GetClientSize(&cw,&ch);
	//wxLogTrace("ClientSize cw=%d ch=%d", cw, ch) ;

  GetSize(&cw,&ch);
	//wxLogTrace("Size cw=%d ch=%d", cw, ch) ;

  int mpx,mpy;
  ::wxGetMousePosition(&mpx,&mpy);
  wbit = mDefaultBitmap->GetWidth();  // NB! Should use largest bm
  hbit = mDefaultBitmap->GetHeight(); // NB! Should use largest bm
  return(mpx >= cx && mpx <= cx + wbit &&
         mpy >= cy && mpy <= cy + hbit);
} 

