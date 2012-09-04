/////////////////////////////////////////////////////////////////////////////
// Name:        LinearMeter.cpp
// Purpose:     wxIndustrialControls Library
// Author:      Marco Cavallini <m.cavallini AT koansoftware.com>
// Modified by: 
// Copyright:   (C)2004-2006 Copyright by Koan s.a.s. - www.koansoftware.com
// Licence:     KWIC License http://www.koansoftware.com/kwic/kwic-license.htm
/////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "kprec.h"		//#include "wx/wxprec.h"

#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include <wx/image.h>
#include <math.h>
#include <wx/log.h>

#include "LinearMeter.h"
#include <wx/event.h>

//IMPLEMENT_DYNAMIC_CLASS(kwxMeter, wxWindow)

BEGIN_EVENT_TABLE(kwxLinearMeter,wxWindow)
	EVT_PAINT(kwxLinearMeter::OnPaint)
	EVT_ERASE_BACKGROUND(kwxLinearMeter::OnEraseBackGround)
END_EVENT_TABLE()

kwxLinearMeter::kwxLinearMeter(wxWindow* parent,
		                   const wxWindowID id,
		                   const wxString& label,
                           const wxPoint& pos,
                           const wxSize& size,
		                   const long int style)
	: wxWindow(parent, id, pos, size, 0)
{

	if (parent)
		SetBackgroundColour(parent->GetBackgroundColour());
	else
		SetBackgroundColour(*wxLIGHT_GREY);

    //SetSize(size);
    SetAutoLayout(TRUE);
	Refresh();

	m_id = id;

	membitmap = new wxBitmap(size.GetWidth(), size.GetHeight()) ;

	m_cActiveBar = wxGREEN;
	m_cPassiveBar = wxWHITE;

	m_cValueColour = wxRED ;
	m_cBorderColour = wxRED ;
	m_cLimitColour = wxBLACK ;
	m_cTagsColour = wxGREEN ;

	m_nScaledVal = 0;
	m_nRealVal = 0;

	m_nTagsNum = 0 ;

	m_nMax = 100 ;
	m_nMin = 0 ;

	m_bDirOrizFlag = true ;
	m_bShowCurrent = true ;
	m_bShowLimits = true ;

}

kwxLinearMeter::~kwxLinearMeter()
{
	delete membitmap;
}

void kwxLinearMeter::SetValue(int val) 
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


void kwxLinearMeter::OnPaint(wxPaintEvent &event)
{
	wxPaintDC dc(this);

	int w,h;
	int yPoint, rectHeight ;

	GetClientSize(&w,&h);

	/////////////////

	// Create a memory DC
	wxMemoryDC temp_dc;
	temp_dc.SelectObject(*membitmap);


	temp_dc.SetBackground(*wxTheBrushList->FindOrCreateBrush(*m_cPassiveBar,wxSOLID));
	temp_dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(*m_cPassiveBar,wxSOLID));
	temp_dc.Clear();


	///////////////////

	temp_dc.SetPen(*wxThePenList->FindOrCreatePen(*m_cBorderColour, 1, wxSOLID));
	temp_dc.DrawRectangle(0, 0, w, h);

	temp_dc.SetPen(*wxThePenList->FindOrCreatePen(*m_cActiveBar, 1, wxSOLID));
	temp_dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(*m_cActiveBar,wxSOLID));

	if(m_bDirOrizFlag)
		temp_dc.DrawRectangle(1, 1, m_nScaledVal, h - 2);
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

		temp_dc.DrawRectangle(1, yPoint, w - 2, rectHeight);
	}

	if (m_bShowCurrent)
		DrawCurrent(temp_dc);	//valore attuale
	
	if (m_bShowLimits)
		DrawLimits(temp_dc);	//valore minimo e massimo
	
	if (m_nTagsNum > 0 )
		DrawTags(temp_dc) ;


	// We can now draw into the memory DC...
	// Copy from this DC to another DC.
	dc.Blit(0, 0, w, h, &temp_dc, 0, 0);


}

void kwxLinearMeter::DrawCurrent(wxDC &temp_dc)
{
	int w,h;
	int tw,th;
	wxString s;

	GetClientSize(&w, &h);

	//valore attuale
	s.Printf("%d", m_nRealVal);
	temp_dc.GetTextExtent(s, &tw, &th);
	temp_dc.SetTextForeground(*m_cValueColour);
	temp_dc.DrawText(s, w / 2 - tw / 2 , h / 2 - th / 2);
}

void kwxLinearMeter::DrawLimits(wxDC &temp_dc)
{
	int w,h;
	int tw,th;
	wxString s;

	GetClientSize(&w, &h);
	
	temp_dc.SetFont(m_Font);
	temp_dc.SetTextForeground(*m_cLimitColour);

	if(m_bDirOrizFlag)
	{
		//valore minimo
		s.Printf("%d", m_nMin);	
		temp_dc.GetTextExtent(s, &tw, &th);
		temp_dc.DrawText(s, 5, h / 2 - th / 2);

		//valore massimo
		s.Printf("%d", m_nMax);
		temp_dc.GetTextExtent(s, &tw, &th);
		temp_dc.DrawText(s,w - tw - 5, h / 2 - th / 2);
	}
	else
	{
		//valore minimo
		s.Printf("%d", m_nMin);	
		temp_dc.GetTextExtent(s, &tw, &th);
		temp_dc.DrawText(s, w / 2 - tw / 2, h - th - 5);

		//valore massimo
		s.Printf("%d", m_nMax);
		temp_dc.GetTextExtent(s, &tw, &th);
		temp_dc.DrawText(s, w / 2 - tw / 2, 5);
	}
	
	

}

void kwxLinearMeter::DrawTags(wxDC &temp_dc)
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

	temp_dc.SetPen(*wxThePenList->FindOrCreatePen(*m_cTagsColour, 1, wxSOLID));
	temp_dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(*m_cTagsColour,wxSOLID));
	temp_dc.SetTextForeground(*m_cTagsColour);

	while (ntag < m_nTagsNum)
	{
		scalval = ceil((m_aTagsVal[ ntag] - m_nMin) * tcoeff);
		text.Printf("%d", m_aTagsVal[ ntag]) ;
		
		if(m_bDirOrizFlag)
		{
			temp_dc.DrawLine(scalval + 1, h - 2 , scalval + 1, h - 10);
			temp_dc.GetTextExtent(text, &tw, &th);
			temp_dc.DrawText(text, scalval + 1 - (tw / 2 ), h - 10 - th);
		}
		else
		{
			temp_dc.DrawLine(w - 2, h - scalval , w - 10 , h - scalval);
			temp_dc.GetTextExtent(text, &tw, &th);
			temp_dc.DrawText(text, w - 10 - tw, h - scalval - (th / 2) );
		}

		ntag++ ;
	}
}


void kwxLinearMeter::AddTag(int val)
{
	m_aTagsVal[m_nTagsNum++] = val ;
}

