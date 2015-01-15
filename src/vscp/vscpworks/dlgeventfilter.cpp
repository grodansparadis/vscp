/////////////////////////////////////////////////////////////////////////////
// Name:        dlgeventfilter.cpp
// Purpose:     
// Author:      Ake Hedman, Paradise of the Frog/Grodans Paradis AB
// Modified by: 
// Created:     13/01/2015 13:00:07
// RCS-ID:      
// Copyright:   Copyright (C) 2014 Grodans Paradis AB
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include "dlgeventfilter.h"

////@begin XPM images
////@end XPM images


/*
 * EventFilter type definition
 */

IMPLEMENT_DYNAMIC_CLASS( EventFilter, wxDialog )


/*
 * EventFilter event table definition
 */

BEGIN_EVENT_TABLE( EventFilter, wxDialog )

////@begin EventFilter event table entries
    EVT_LISTBOX( ID_LISTBOX_DISPLAY, EventFilter::OnListboxDisplaySelected )
    EVT_LISTBOX_DCLICK( ID_LISTBOX_DISPLAY, EventFilter::OnListboxDisplayDoubleClicked )
    EVT_BUTTON( ID_BUTTON5_ADD_DISPLAY_EVENT, EventFilter::OnButton5AddDisplayEventClick )
    EVT_LISTBOX( ID_LISTBOX_FILTER, EventFilter::OnListboxFilterSelected )
    EVT_LISTBOX_DCLICK( ID_LISTBOX_FILTER, EventFilter::OnListboxFilterDoubleClicked )
    EVT_BUTTON( ID_BUTTON_ADD_FILTER_EVENT, EventFilter::OnButtonAddFilterEventClick )
    EVT_RADIOBUTTON( ID_RADIOBUTTON_DISPLAY, EventFilter::OnRadiobuttonDisplaySelected )
    EVT_RADIOBUTTON( ID_RADIOBUTTON_FILTER, EventFilter::OnRadiobuttonFilterSelected )
    EVT_CHECKBOX( ID_CHECKBOX_ENABLE, EventFilter::OnCheckboxEnableClick )
    EVT_BUTTON( ID_BUTTON_OK, EventFilter::OnButtonOkClick )
    EVT_BUTTON( ID_BUTTON_CANCEL, EventFilter::OnButtonCancelClick )
    EVT_BUTTON( ID_BUTTON_LOAD, EventFilter::OnButtonLoadClick )
    EVT_BUTTON( ID_BUTTON_SAVE, EventFilter::OnButtonSaveClick )
////@end EventFilter event table entries

END_EVENT_TABLE()


/*
 * EventFilter constructors
 */

EventFilter::EventFilter()
{
    Init();
}

EventFilter::EventFilter( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*
 * EventFilter creator
 */

bool EventFilter::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin EventFilter creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end EventFilter creation
    return true;
}


/*
 * EventFilter destructor
 */

EventFilter::~EventFilter()
{
////@begin EventFilter destruction
////@end EventFilter destruction
}


/*
 * Member initialisation
 */

void EventFilter::Init()
{
////@begin EventFilter member initialisation
    m_ctrlListDisplay = NULL;
    m_ctrlListFilter = NULL;
    m_ctrlRadioDisplay = NULL;
    m_ctrlRadioFilter = NULL;
    m_ctrlCheckBoxEnable = NULL;
////@end EventFilter member initialisation
}


/*
 * Control creation for EventFilter
 */

void EventFilter::CreateControls()
{    
////@begin EventFilter content construction
    EventFilter* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemDialog1, wxID_STATIC, _("Display event(s) in this list"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText3->SetFont(wxFont(12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString m_ctrlListDisplayStrings;
    m_ctrlListDisplay = new wxListBox( itemDialog1, ID_LISTBOX_DISPLAY, wxDefaultPosition, wxDefaultSize, m_ctrlListDisplayStrings, wxLB_SINGLE );
    if (EventFilter::ShowToolTips())
        m_ctrlListDisplay->SetToolTip(_("Right click to add/remove/clone"));
    itemBoxSizer2->Add(m_ctrlListDisplay, 0, wxGROW|wxALL, 5);

    wxButton* itemButton5 = new wxButton( itemDialog1, ID_BUTTON5_ADD_DISPLAY_EVENT, _("Add display event..."), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton5->SetDefault();
    if (EventFilter::ShowToolTips())
        itemButton5->SetToolTip(_("Add event that should be displayed"));
    itemBoxSizer2->Add(itemButton5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    itemBoxSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemDialog1, wxID_STATIC, _("Filter away event(s) in this list"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText7->SetFont(wxFont(12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Tahoma")));
    itemBoxSizer2->Add(itemStaticText7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString m_ctrlListFilterStrings;
    m_ctrlListFilter = new wxListBox( itemDialog1, ID_LISTBOX_FILTER, wxDefaultPosition, wxDefaultSize, m_ctrlListFilterStrings, wxLB_SINGLE );
    if (EventFilter::ShowToolTips())
        m_ctrlListFilter->SetToolTip(_("Right click to add/remove/clone"));
    itemBoxSizer2->Add(m_ctrlListFilter, 0, wxGROW|wxALL, 5);

    wxButton* itemButton9 = new wxButton( itemDialog1, ID_BUTTON_ADD_FILTER_EVENT, _("Add filter event..."), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton9->SetDefault();
    if (EventFilter::ShowToolTips())
        itemButton9->SetToolTip(_("Add event to filter out"));
    itemBoxSizer2->Add(itemButton9, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer10, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_ctrlRadioDisplay = new wxRadioButton( itemDialog1, ID_RADIOBUTTON_DISPLAY, _("Display"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ctrlRadioDisplay->SetValue(false);
    if (EventFilter::ShowToolTips())
        m_ctrlRadioDisplay->SetToolTip(_("Show only events in display list"));
    itemBoxSizer10->Add(m_ctrlRadioDisplay, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ctrlRadioFilter = new wxRadioButton( itemDialog1, ID_RADIOBUTTON_FILTER, _("Filter"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ctrlRadioFilter->SetValue(true);
    if (EventFilter::ShowToolTips())
        m_ctrlRadioFilter->SetToolTip(_("Filter away events in filter list"));
    itemBoxSizer10->Add(m_ctrlRadioFilter, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer10->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ctrlCheckBoxEnable = new wxCheckBox( itemDialog1, ID_CHECKBOX_ENABLE, _("Enable"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ctrlCheckBoxEnable->SetValue(false);
    if (EventFilter::ShowToolTips())
        m_ctrlCheckBoxEnable->SetToolTip(_("Enable/disable filter"));
    itemBoxSizer10->Add(m_ctrlCheckBoxEnable, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer15 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer15, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton16 = new wxButton( itemDialog1, ID_BUTTON_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton16->SetDefault();
    itemBoxSizer15->Add(itemButton16, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton17 = new wxButton( itemDialog1, ID_BUTTON_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton17->SetDefault();
    itemBoxSizer15->Add(itemButton17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer15->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton19 = new wxButton( itemDialog1, ID_BUTTON_LOAD, _("Load"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton19->SetDefault();
    if (EventFilter::ShowToolTips())
        itemButton19->SetToolTip(_("Load filter set from disk"));
    itemBoxSizer15->Add(itemButton19, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton20 = new wxButton( itemDialog1, ID_BUTTON_SAVE, _("Save"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton20->SetDefault();
    if (EventFilter::ShowToolTips())
        itemButton20->SetToolTip(_("Save filter set to disk"));
    itemBoxSizer15->Add(itemButton20, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Connect events and objects
    m_ctrlListDisplay->Connect(ID_LISTBOX_DISPLAY, wxEVT_RIGHT_DOWN, wxMouseEventHandler(EventFilter::OnRightDown), NULL, this);
    m_ctrlListFilter->Connect(ID_LISTBOX_FILTER, wxEVT_RIGHT_DOWN, wxMouseEventHandler(EventFilter::OnRightDown), NULL, this);
////@end EventFilter content construction
}


/*
 * Should we show tooltips?
 */

bool EventFilter::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap EventFilter::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin EventFilter bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end EventFilter bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon EventFilter::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin EventFilter icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end EventFilter icon retrieval
}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON5_ADD_DISPLAY_EVENT
 */

void EventFilter::OnButton5AddDisplayEventClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON5_ADD_DISPLAY_EVENT in EventFilter.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON5_ADD_DISPLAY_EVENT in EventFilter. 
}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_ADD_FILTER_EVENT
 */

void EventFilter::OnButtonAddFilterEventClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_ADD_FILTER_EVENT in EventFilter.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_ADD_FILTER_EVENT in EventFilter. 
}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_OK
 */

void EventFilter::OnButtonOkClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_OK in EventFilter.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_OK in EventFilter. 
}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_CANCEL
 */

void EventFilter::OnButtonCancelClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_CANCEL in EventFilter.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_CANCEL in EventFilter. 
}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_LOAD
 */

void EventFilter::OnButtonLoadClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_LOAD in EventFilter.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_LOAD in EventFilter. 
}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SAVE
 */

void EventFilter::OnButtonSaveClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SAVE in EventFilter.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SAVE in EventFilter. 
}


/*
 * wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_LISTBOX_DISPLAY
 */

void EventFilter::OnListboxDisplaySelected( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_LISTBOX_DISPLAY in EventFilter.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_LISTBOX_DISPLAY in EventFilter. 
}


/*
 * wxEVT_COMMAND_LISTBOX_DOUBLECLICKED event handler for ID_LISTBOX_DISPLAY
 */

void EventFilter::OnListboxDisplayDoubleClicked( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_LISTBOX_DOUBLECLICKED event handler for ID_LISTBOX_DISPLAY in EventFilter.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_LISTBOX_DOUBLECLICKED event handler for ID_LISTBOX_DISPLAY in EventFilter. 
}


/*
 * wxEVT_RIGHT_DOWN event handler for ID_LISTBOX_DISPLAY
 */

void EventFilter::OnRightDown( wxMouseEvent& event )
{
////@begin wxEVT_RIGHT_DOWN event handler for ID_LISTBOX_DISPLAY in EventFilter.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_RIGHT_DOWN event handler for ID_LISTBOX_DISPLAY in EventFilter. 
}


/*
 * wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_LISTBOX_FILTER
 */

void EventFilter::OnListboxFilterSelected( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_LISTBOX_FILTER in EventFilter.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_LISTBOX_FILTER in EventFilter. 
}


/*
 * wxEVT_COMMAND_LISTBOX_DOUBLECLICKED event handler for ID_LISTBOX_FILTER
 */

void EventFilter::OnListboxFilterDoubleClicked( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_LISTBOX_DOUBLECLICKED event handler for ID_LISTBOX_FILTER in EventFilter.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_LISTBOX_DOUBLECLICKED event handler for ID_LISTBOX_FILTER in EventFilter. 
}


/*
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX_ENABLE
 */

void EventFilter::OnCheckboxEnableClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX_ENABLE in EventFilter.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX_ENABLE in EventFilter. 
}


/*
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON_FILTER
 */

void EventFilter::OnRadiobuttonFilterSelected( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON_FILTER in EventFilter.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON_FILTER in EventFilter. 
}


/*
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON_DISPLAY
 */

void EventFilter::OnRadiobuttonDisplaySelected( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON_DISPLAY in EventFilter.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON_DISPLAY in EventFilter. 
}

