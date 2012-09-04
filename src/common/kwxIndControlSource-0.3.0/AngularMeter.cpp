/////////////////////////////////////////////////////////////////////////////
// Name:        AngularMeter.cpp
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

#include "AngularMeter.h"
#include <wx/event.h>


//IMPLEMENT_DYNAMIC_CLASS(kwxAngularMeter, wxWindow)

BEGIN_EVENT_TABLE(kwxAngularMeter,wxWindow)
	EVT_PAINT(kwxAngularMeter::OnPaint)
	EVT_ERASE_BACKGROUND(kwxAngularMeter::OnEraseBackGround)
END_EVENT_TABLE()

kwxAngularMeter::kwxAngularMeter(wxWindow* parent,
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

	//valori di default

	m_nScaledVal = 0;		//gradi
	m_nRealVal = 0;
	m_nTick = 0;	//numero tacche
	m_nSec = 1;		//default numero settori
	m_nRangeStart = 0;
	m_nRangeEnd = 220;
	m_nAngleStart = -20;
	m_nAngleEnd = 200;
	m_aSectorColor[0] = *wxWHITE;
//	m_cBackColour = *wxLIGHT_GREY;
	m_cBackColour = GetBackgroundColour() ;		//default sfondo applicazione
	m_cNeedleColour = *wxRED;	//indicatore
	m_cBorderColour = GetBackgroundColour() ;
	m_dPI = 4.0 * atan(1.0);
	m_Font = *wxSWISS_FONT;	//font
	m_bDrawCurrent = true ;

	membitmap = new wxBitmap(size.GetWidth(), size.GetHeight()) ;



	/////////////// TODO : Test for BMP image loading /////////////////
/*

	m_pPreviewBmp = NULL ;

    wxBitmap bitmap( 300, 300 );
    wxImage image = bitmap.ConvertToImage();

    image.Destroy();

    if ( !image.LoadFile( "thumbnail.bmp", wxBITMAP_TYPE_BMP ) )
        wxLogError(wxT("Can't load BMP image"));
    else
        m_pPreviewBmp = new wxBitmap( image );
*/

}

kwxAngularMeter::~kwxAngularMeter()
{
	delete membitmap;
}

void kwxAngularMeter::SetValue(int val) 
{ 
	int deltarange = m_nRangeEnd - m_nRangeStart;
//	int rangezero = deltarange - m_nRangeStart;
	int deltaangle = m_nAngleEnd - m_nAngleStart;
	double coeff = (double)deltaangle / (double)deltarange;

	m_nScaledVal = (int)((double)(val - m_nRangeStart) * coeff); 
	m_nRealVal = val;
	Refresh();
}


void kwxAngularMeter::OnPaint(wxPaintEvent &event)
{
	wxPaintDC old_dc(this);
	
	int w,h ;
	
	GetClientSize(&w,&h);

	/////////////////


	// Create a memory DC
	wxMemoryDC dc;
	dc.SelectObject(*membitmap);

	dc.SetBackground(*wxTheBrushList->FindOrCreateBrush(m_cBackColour,wxSOLID));
	dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(m_cBackColour,wxSOLID));
	dc.Clear();


/*
    if (m_pPreviewBmp && m_pPreviewBmp->Ok())
		dc.DrawBitmap( *m_pPreviewBmp, 1, 1 );
*/


	///////////////////

	//Rettangolo

	dc.SetPen(*wxThePenList->FindOrCreatePen(m_cBorderColour, 1, wxSOLID));
	dc.DrawRectangle(0,0,w,h);
	
	//settori
	DrawSectors(dc) ;

	//tacche
	if (m_nTick > 0)
		DrawTicks(dc);

	//indicatore lancetta

	DrawNeedle(dc);

	
	//testo valore
	if (m_bDrawCurrent) 
	{
		wxString valuetext;
		valuetext.Printf("%d",m_nRealVal);
		dc.SetFont(m_Font);
		dc.DrawText(valuetext, (w / 2) - 10, (h / 2) + 10);
	}

	// We can now draw into the memory DC...
	// Copy from this DC to another DC.
	old_dc.Blit(0, 0, w, h, &dc, 0, 0);
}


void kwxAngularMeter::DrawNeedle(wxDC &dc) 
{
	//indicatore triangolare
	double dxi,dyi, val;
	wxPoint ppoint[6];
	int w, h ;

	GetClientSize(&w,&h);

	dc.SetPen(*wxThePenList->FindOrCreatePen(m_cNeedleColour, 1,wxSOLID));

	val = (m_nScaledVal + m_nAngleStart) * m_dPI / 180; //radianti parametro angolo

	dyi = sin(val - 90) * 2; //coordinate base sinistra
	dxi = cos(val - 90) * 2;

	ppoint[0].x = (w / 2) - dxi;	//base sinistra
	ppoint[0].y = (h / 2) - dyi;

	dxi = cos(val) * ((h / 2) - 4); //coordinate punta indicatore
	dyi = sin(val) * ((h / 2) - 4);

	ppoint[2].x = (w / 2) - dxi;	//punta
	ppoint[2].y = (h / 2) - dyi;

	dxi = cos(val + 90) * 2; //coordinate base destra
	dyi = sin(val + 90) * 2;

	ppoint[4].x = (w / 2) - dxi;	//base destra
	ppoint[4].y = (h / 2) - dyi;

	ppoint[5].x = ppoint[0].x;	//ritorno base sinistra
	ppoint[5].y = ppoint[0].y;

//////////////////////////
	val = (m_nScaledVal + m_nAngleStart + 1) * m_dPI / 180;

	dxi = cos(val) * ((h / 2) - 10); //coordinate medio destra
	dyi = sin(val) * ((h / 2) - 10);

	ppoint[3].x = (w / 2) - dxi;	//base media destra
	ppoint[3].y = (h / 2) - dyi;

	val = (m_nScaledVal + m_nAngleStart - 1) * m_dPI / 180;

	dxi = cos(val) * ((h / 2) - 10); //coordinate medio sinistra
	dyi = sin(val) * ((h / 2) - 10);

	ppoint[1].x = (w / 2) - dxi;	//base media sinistra
	ppoint[1].y = (h / 2) - dyi;

/////////////////////////


	dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(m_cNeedleColour,wxSOLID));

	dc.DrawPolygon(6, ppoint, 0, 0, wxODDEVEN_RULE);

	//cerchio indicatore
	dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(*wxWHITE,wxSOLID));
	dc.DrawCircle(w / 2, h / 2, 4);
}



void kwxAngularMeter::DrawSectors(wxDC &dc)
{
	double starc,endarc;
	int secount,dx,dy;
	int w,h ;

	double val;
	
	GetClientSize(&w,&h);

	//arco -> settori
	dc.SetPen(*wxThePenList->FindOrCreatePen(*wxBLACK, 1, wxSOLID));

	starc = m_nAngleStart;
	endarc = starc + ((m_nAngleEnd - m_nAngleStart) / (double)m_nSec);
	//dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(*wxRED,wxSOLID));
	for(secount=0;secount<m_nSec;secount++)
	{
		dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(m_aSectorColor[secount],wxSOLID));
		dc.DrawEllipticArc(0,0,w,h,180 - endarc,180 - starc);
		//dc.DrawEllipticArc(0,0,w,h,0,180);
		starc = endarc;
		endarc += ((m_nAngleEnd - m_nAngleStart) / (double)m_nSec);
	}

	val = (m_nAngleStart * m_dPI) / 180.0;
	dx = cos(val) * h / 2.0;
	dy = sin(val) * h / 2.0;

	dc.DrawLine(w / 2, h / 2, (w / 2) - dx, (h / 2) - dy);	//linea sinistra

	val = (m_nAngleEnd * m_dPI) / 180.0;
	dx = cos(val) * h / 2.0;
	dy = sin(val) * h / 2.0;
		
	dc.DrawLine(w / 2, h / 2, (w / 2) - dx, (h / 2) - dy);	//linea destra

}

void kwxAngularMeter::DrawTicks(wxDC &dc)
{
	double intervallo = (m_nAngleEnd - m_nAngleStart) / (m_nTick + 1.0);
	double valint = intervallo + m_nAngleStart;
	double tx, ty;
	double val;
	double dx, dy;
	int n, w, h;
	int tw, th;
	wxString s;
	
	GetClientSize(&w, &h);

	for(n = 0;n < m_nTick;n++)
	{
		val=(valint * m_dPI) / 180;
		//wxLogTrace("v: %f",valint);
		dx = cos(val) * (h/2);	//punto sul cerchio
		dy = sin(val) * (h/2);

		tx = cos(val) * ((h / 2) - 10);	//punto nel cerchio
		ty = sin(val) * ((h / 2) - 10);

		dc.DrawLine((w / 2) - tx, (h / 2) - ty, (w / 2) - dx, (h / 2) - dy);
		
		int deltarange = m_nRangeEnd - m_nRangeStart;
		int deltaangle = m_nAngleEnd - m_nAngleStart;
		double coeff = (double)deltaangle / (double)deltarange;

		int rightval = (int)(((valint - (double)m_nAngleStart)) / coeff) + m_nRangeStart;
		
		s.Printf("%d", rightval);

		dc.GetTextExtent(s, &tw, &th);

		val = ((valint - 4) * m_dPI) / 180;	//angolo spostato
		tx = cos(val) * ((h / 2) - 12);	//punto testo
		ty = sin(val) * ((h / 2) - 12);

		dc.SetFont(m_Font);

		dc.DrawRotatedText(s,(w / 2) - tx, (h / 2) - ty, 90 - valint);

		valint = valint + intervallo;
	}	

	
}

void kwxAngularMeter::SetSectorColor(int nSector, wxColour colour) 
{ 
	m_aSectorColor[nSector] = colour; 
}
