/////////////////////////////////////////////////////////////////////////////
// Name:        LinearRegulator.cpp
// Purpose:     wxIndustrialControls Library
// Author:      Marco Cavallini <m.cavallini AT koansoftware.com>
// Modified by: 
// Copyright:   (C)2004-2006 Copyright by Koan s.a.s. - www.koansoftware.com
// Licence:     KWIC License http://www.koansoftware.com/kwic/kwic-license.htm
/////////////////////////////////////////////////////////////////////////////


#include "kprec.h"		//#include "wx/wxprec.h"

#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "LinearRegulator.h"
#include <wx/event.h>
#include <math.h>

IMPLEMENT_DYNAMIC_CLASS(kwxLinearRegulator, wxControl)

BEGIN_EVENT_TABLE(kwxLinearRegulator,wxControl)
	EVT_MOUSE_EVENTS(kwxLinearRegulator::OnMouse)
	EVT_PAINT(kwxLinearRegulator::OnPaint)
	EVT_ERASE_BACKGROUND(kwxLinearRegulator::OnEraseBackGround)
END_EVENT_TABLE()

kwxLinearRegulator::kwxLinearRegulator(wxWindow* parent,
		                   const wxWindowID id,
		                   const wxString& label,
                           const wxPoint& pos,
                           const wxSize& size,
		                   const long int style)
	: wxControl(parent, id, pos, size, style)
{
	
	m_id = id;

	m_ControlSize = size ;

	m_nScaledVal = 0;
	m_nRealVal = 0;
	m_nTagsNum = 0 ;
	
	m_nStato = 0 ;
	m_nMin = 0 ;
	m_nMax = 100 ;

	m_bDirOrizFlag = true ;
	m_bShowCurrent = true ;
	m_bShowLimits= true ;

	m_cActiveBar = wxGREEN;
	m_cPassiveBar = wxWHITE;
	m_cBorderColour = wxRED ;
	m_cTagsColour = wxBLACK ;
	m_cValueColour = wxRED ;
	m_cLimitColour = wxBLACK ;

	m_bShowLimits = true ;
	m_bShowCurrent = true ;

	membitmap = new wxBitmap(size.GetWidth(), size.GetHeight()) ;
}

kwxLinearRegulator::~kwxLinearRegulator()
{
	if (membitmap)
		delete membitmap ;
}


void kwxLinearRegulator::OnPaint(wxPaintEvent &event)
{
	wxPaintDC old_dc(this);
	
	int w,h;
	int yPoint, rectHeight ;
	
	GetClientSize(&w,&h);

	wxMemoryDC dc;
	dc.SelectObject(*membitmap);


	dc.SetBackground(*wxTheBrushList->FindOrCreateBrush(*m_cPassiveBar,wxSOLID));
	dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(*m_cPassiveBar,wxSOLID));
	dc.Clear();
	///////////////////////////////////////////////////////////////////////////

	dc.SetPen(*wxThePenList->FindOrCreatePen(*m_cBorderColour, 1, wxSOLID));
	dc.DrawRectangle(0, 0, w, h);

	dc.SetPen(*wxThePenList->FindOrCreatePen(*m_cActiveBar, 1, wxSOLID));
	dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(*m_cActiveBar,wxSOLID));

	
	if(m_bDirOrizFlag)
		dc.DrawRectangle(1, 1, m_nScaledVal, h - 2);
	else
	{
		yPoint = h - m_nScaledVal ;

		if (m_nScaledVal == 0 )
			rectHeight = m_nScaledVal ;
		else
		{
			if (m_nRealVal == m_nMax)
			{
				rectHeight = m_nScaledVal;
				yPoint -= 1 ;
			}
			else
				rectHeight = m_nScaledVal - 1 ;
		}

		dc.DrawRectangle(1, yPoint, w - 2, rectHeight);
	}

	if (m_bShowCurrent)
		DrawCurrent(dc);	//valore attuale
	
	if (m_bShowLimits)
		DrawLimits(dc);	//valore minimo e massimo
	
	if (m_nTagsNum > 0 )
		DrawTags(dc) ;


	// We can now draw into the memory DC...
	// Copy from this DC to another DC.
	old_dc.Blit(0, 0, w, h, &dc, 0, 0);

}

void kwxLinearRegulator::OnMouse(wxMouseEvent& event)
{
	if (m_nStato == 0 && event.Entering())	//dentro nel controllo
		m_nStato = 1 ;
	else if (m_nStato >= 1 && event.Leaving())	//fuori dal controllo
		m_nStato = 0 ;
	else if (m_nStato == 1 && event.LeftDown())	//click o inizio trascinamento
	{
//		wxLogTrace("Mouse Premuto") ;
		m_nStato = 2 ;
		m_mousePosition = event.GetPosition();
		DrawPosition() ;
	}
	else if (m_nStato == 2 && event.LeftIsDown())	//trascinamento
	{
//		wxLogTrace("Mouse trascinato") ;
		m_mousePosition = event.GetPosition();
		DrawPosition() ;
	}
	else if (m_nStato == 2 && event.LeftUp())	//fine trascinamento o rilascio click
	{
//		wxLogTrace("Mouse rilasciato") ;
		m_nStato = 1 ;
	}

}



void kwxLinearRegulator::DrawPosition()
{
	double coeff = 0 ;
	bool draw = true ;

	if (m_bDirOrizFlag)
	{
		m_nScaledVal = m_mousePosition.x ;
		if ( m_nScaledVal < 0 || m_nScaledVal >= m_ControlSize.GetWidth() - 1)
			draw = false ;
		else
			coeff = (m_ControlSize.GetWidth() - 2) / (double)(m_nMax - m_nMin);
	}
	else
	{
		m_nScaledVal = m_ControlSize.GetHeight() - m_mousePosition.y;
		if ( m_nScaledVal < 0 || m_nScaledVal >= m_ControlSize.GetHeight() - 1)
			draw = false ;
		else
			coeff = (m_ControlSize.GetHeight() - 2) / (double)(m_nMax - m_nMin);
	}


	if (m_nRealVal <= m_nMax && draw)
	{
		m_nRealVal = floor(m_nScaledVal / coeff) ;
		Refresh() ;
		Change() ;
	}

}


void kwxLinearRegulator::AddTag(int val)
{
	m_aTagsVal[m_nTagsNum++] = val ;
}

//evento cambio valore
void kwxLinearRegulator::Change()
{
	wxCommandEvent event(kwxEVT_LINEARREG_CHANGE, GetId());
	event.SetEventObject(this);
	ProcessCommand(event) ;
}

//per inizializzazione
void kwxLinearRegulator::SetVal(int val) 
{ 
	int w,h;
	double coeff;

	GetClientSize(&w,&h);

	if(m_bDirOrizFlag)
		coeff = (w - 2) / (double)(m_nMax - m_nMin);
	else
		coeff = (h - 2) / (double)(m_nMax - m_nMin);

	m_nScaledVal = ceil((val - m_nMin) * coeff);
	m_nRealVal = val;

	Refresh();
}

///////////////////////////////
//	FUNZIONI DISEGNO CUSTOM	 //		
///////////////////////////////

//Disegna valore corrente
void kwxLinearRegulator::DrawCurrent(wxDC &dc)
{
	int w,h;
	int tw,th;
	wxString s;

	GetClientSize(&w, &h);

	//valore attuale
	s.Printf("%d", m_nRealVal);
	dc.GetTextExtent(s, &tw, &th);
	dc.SetTextForeground(*m_cValueColour);
	dc.DrawText(s, w / 2 - tw / 2 , h / 2 - th / 2);
}

//Disegna limiti controllo
void kwxLinearRegulator::DrawLimits(wxDC &dc)
{
	int w,h;
	int tw,th;
	wxString s;

	GetClientSize(&w, &h);
	
	dc.SetFont(m_Font);
	dc.SetTextForeground(*m_cLimitColour);

	if(m_bDirOrizFlag)
	{
		//valore minimo
		s.Printf("%d", m_nMin);	
		dc.GetTextExtent(s, &tw, &th);
		dc.DrawText(s, 5, h / 2 - th / 2);

		//valore massimo
		s.Printf("%d", m_nMax);
		dc.GetTextExtent(s, &tw, &th);
		dc.DrawText(s,w - tw - 5, h / 2 - th / 2);
	}
	else
	{
		//valore minimo
		s.Printf("%d", m_nMin);	
		dc.GetTextExtent(s, &tw, &th);
		dc.DrawText(s, w / 2 - tw / 2, h - th - 5);

		//valore massimo
		s.Printf("%d", m_nMax);
		dc.GetTextExtent(s, &tw, &th);
		dc.DrawText(s, w / 2 - tw / 2, 5);
	}

}

//Disegna i tag impostati
void kwxLinearRegulator::DrawTags(wxDC &dc)
{
	int ntag = 0 ;
	int w, h ;
	int tw,th;
	int scalval = 0 ;
	double tcoeff ;

	wxString text ;

	GetClientSize(&w,&h);
	if(m_bDirOrizFlag)
		tcoeff = (w - 2) / (double)(m_nMax - m_nMin);
	else
		tcoeff = (h - 2) / (double)(m_nMax - m_nMin);

	dc.SetPen(*wxThePenList->FindOrCreatePen(*m_cTagsColour, 1, wxSOLID));
	dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(*m_cTagsColour,wxSOLID));
	dc.SetTextForeground(*m_cTagsColour);

	while (ntag < m_nTagsNum)
	{
		scalval = floor((m_aTagsVal[ ntag] - m_nMin) * tcoeff);
		text.Printf("%d", m_aTagsVal[ ntag]) ;
		
		if(m_bDirOrizFlag)
		{
			dc.DrawLine(scalval + 1, h - 2 , scalval + 1, h - 10);
			dc.GetTextExtent(text, &tw, &th);
			dc.DrawText(text, scalval + 1 - (tw / 2 ), h - 10 - th);
		}
		else
		{
			dc.DrawLine(w - 2, h - scalval , w - 10 , h - scalval);
			dc.GetTextExtent(text, &tw, &th);
			dc.DrawText(text, w - 10 - tw, h - scalval - (th / 2) );
		}

		ntag++ ;
	}
}


