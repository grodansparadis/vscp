/////////////////////////////////////////////////////////////////////////////
// Name:        NewBitmapButton.cpp
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

#include "BitmapButton.h"
#include <wx/event.h>



IMPLEMENT_DYNAMIC_CLASS(kwxBitmapButton, wxControl)


BEGIN_EVENT_TABLE(kwxBitmapButton,wxControl)
  EVT_PAINT(kwxBitmapButton::OnPaint)
  EVT_MOUSE_EVENTS(kwxBitmapButton::OnMouse)
  EVT_ERASE_BACKGROUND(kwxBitmapButton::OnEraseBackGround)
  //EVT_MOTION(kwxBitmapButton::OnMouse) 
END_EVENT_TABLE()

kwxBitmapButton::kwxBitmapButton(wxWindow* parent,
		                   const wxWindowID id,
		                   const wxString& label,
                           wxBitmap& defaultBitmap,
							wxBitmap& MoveMouseBitmap,
							wxBitmap& PressMouseBitmap,
                           const wxPoint& pos,
                           const wxSize& size,
		                   const long int style)
	: wxControl(parent, id, pos, size, 0)
{

  if (parent)
    SetBackgroundColour(parent->GetBackgroundColour());
  else
    SetBackgroundColour(*wxLIGHT_GREY);

	mDefaultBitmap  = &defaultBitmap;
	mMoveMouseBitmap = &MoveMouseBitmap;
	mPressMouseBitmap = &PressMouseBitmap;
	m_id = id;

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
	m_stato = 0;
	m_oldstato = 0;

	membitmap = new wxBitmap(total_width, total_height) ;

}

void kwxBitmapButton::SetLabel(wxString label)
{
	mLabelStr = label;
} 

void kwxBitmapButton::OnPaint(wxPaintEvent& event)
{
	wxPaintDC old_dc(this);
	


	int w,h;
	
	GetClientSize(&w,&h);

	/////////////////


	// Create a memory DC
	wxMemoryDC dc;
	dc.SelectObject(*membitmap);


	dc.SetBackground(*wxTheBrushList->FindOrCreateBrush(GetBackgroundColour(),wxSOLID));
	dc.Clear();



	///////////////////

	
	// se impostato n bitmap lo disegno
    //if (mDefaultBitmap)
	if(m_stato == 0)
		dc.DrawBitmap(*mDefaultBitmap, 0, 0, TRUE);
	else if(m_stato == 1)
		dc.DrawBitmap(*mMoveMouseBitmap, 0, 0, TRUE);
	else if(m_stato == 2)
		dc.DrawBitmap(*mPressMouseBitmap, 0, 0, TRUE);

	// Cornice intorno
	dc.SetPen(*wxThePenList->FindOrCreatePen(*wxRED, 1, wxSOLID));
	dc.DrawLine(0, 0, 0, h - 1);
	dc.DrawLine(0, 0, w, 0);
	dc.DrawLine(0, h - 1, w, h - 1);
	dc.DrawLine(w - 1, 0, w - 1, h - 1);


	// We can now draw into the memory DC...
	// Copy from this DC to another DC.
	old_dc.Blit(0, 0, w, h, &dc, 0, 0);
 
}


void kwxBitmapButton::OnMouse(wxMouseEvent& event)
{
	if (m_stato == 0 && event.Entering())
		m_stato = 1;	// mouse sul bottone	
	else if (m_stato == 1 && event.LeftDown())
		m_stato = 2;	// uscita click sul bottone
	else if (m_stato >= 1 && event.Leaving())
		m_stato = 0;	// uscita mouse dal bottone
	else if (m_stato == 2 && event.LeftUp())
	{
		Click();		// rilascio sul bottone genera evento
		m_stato = 1;
	}


	if(m_oldstato != m_stato)
		Refresh();
	m_oldstato=m_stato;

	event.Skip();
}




void kwxBitmapButton::Click()
{
//			wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, GetID()) ;
//			ProcessEvent(event) ;

//			wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, GetID()) ;
//			InitCommandEvent(event);
//			Command(event);

	wxCommandEvent event(kwxEVT_BITBUTTON_CLICK, GetId());
	event.SetEventObject(this);
	ProcessCommand(event);

}



/*
void kwxBitmapButton::OnMouse(wxMouseEvent& event)
{
	if(MouseIsOnButton())
	{
		if (event.LeftIsDown())
		{
			m_stato=2;	//bottone premuto
			Click() ;
		}
		else
		{
			m_stato=1;	//mouse sul bottone	
		}
	}
	if(event.Leaving())
	{
		m_stato=0;	//uscita mouse dal bottone
	}

	if(m_oldstato!=m_stato)
	{
		Refresh();
	}

	m_oldstato=m_stato;

	event.Skip();
}

bool kwxBitmapButton::MouseIsOnButton()
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
*/
