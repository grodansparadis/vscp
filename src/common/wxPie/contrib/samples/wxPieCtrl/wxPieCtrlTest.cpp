#include <wx/wx.h>
#include <wx/image.h>
#include <wx/filename.h>
#include <wx/sysopt.h>
#include <wx/wxPieCtrl/wxPieCtrl.h>

class MyTimer;


class MyApp : public wxApp
{
public:
	virtual bool OnInit();
};

DECLARE_APP(MyApp)
IMPLEMENT_APP(MyApp)

class MyFrame : public wxFrame
{
	MyTimer * m_Timer;
public:
	wxPieCtrl * m_Pie;
	wxProgressPie * m_ProgressPie;
	wxSlider * m_Slider;
	wxSlider * m_AngleSlider;
	MyFrame(wxWindow * parent);
	~MyFrame();
	DECLARE_EVENT_TABLE()	
	void OnSlider(wxScrollEvent & event);
	void OnAngleSlider(wxScrollEvent & event);
	void OnToggleTransparency(wxCommandEvent & event);
	void OnToggleEdges(wxCommandEvent & event);
};

class MyTimer : public wxTimer
{
	MyFrame * m_Frame;
	int incr;
public:
	MyTimer(MyFrame * frame) : wxTimer(), m_Frame(frame), incr(1) {}
	void Notify() 
	{ 
		if(m_Frame->m_ProgressPie->GetValue() <= 0) incr = 1;
		if(m_Frame->m_ProgressPie->GetValue() >= m_Frame->m_ProgressPie->GetMaxValue())	incr = -1;		
		m_Frame->m_ProgressPie->SetValue(m_Frame->m_ProgressPie->GetValue()+incr);
		m_Frame->m_ProgressPie->Refresh();
	}
};

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_COMMAND_SCROLL(10002, MyFrame::OnSlider)
EVT_COMMAND_SCROLL(10003, MyFrame::OnAngleSlider)
EVT_BUTTON(10004, MyFrame::OnToggleTransparency)
EVT_BUTTON(10005, MyFrame::OnToggleEdges)
END_EVENT_TABLE()

MyFrame::MyFrame(wxWindow * parent)
: wxFrame(parent, -1, _("Test"), wxPoint(100,100), wxSize(400,500))
{		
	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
	m_Pie = new wxPieCtrl(this, 10001, wxDefaultPosition, wxSize(180,270));
	
	m_Pie->GetLegend()->SetTransparent(true);
	m_Pie->GetLegend()->SetHorBorder(10);
	m_Pie->GetLegend()->SetWindowStyle(wxSTATIC_BORDER);
	m_Pie->GetLegend()->SetLabelFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL , false, wxT("Courier New")));
	m_Pie->GetLegend()->SetLabelColour(wxColour(0,0,127));	
	
	m_Pie->SetBackground(
		wxBitmap(wxFileName(wxGetApp().argv[0]).GetPath()+
				wxFileName::GetPathSeparator()+
				wxT("background.png"), wxBITMAP_TYPE_PNG));
	m_Pie->SetHeight(30);

	wxPiePart part;
	
	part.SetLabel(wxT("SeriesLabel_1"));
	part.SetValue(300);
	part.SetColour(wxColour(200,50,50));
	m_Pie->m_Series.Add(part);
	
	part.SetLabel(wxT("Series Label 2"));
	part.SetValue(200);
	part.SetColour(wxColour(50,200,50));
	m_Pie->m_Series.Add(part);

	part.SetLabel(wxT("HelloWorld Label 3"));
	part.SetValue(50);
	part.SetColour(wxColour(50,50,200));
	m_Pie->m_Series.Add(part);

	m_ProgressPie = new wxProgressPie(this, 10004, 100, 50, wxDefaultPosition, wxSize(180, 200),wxSIMPLE_BORDER);

	m_ProgressPie->SetBackColour(wxColour(150,200,255));
	m_ProgressPie->SetFilledColour(wxColour(0,0,127));
	m_ProgressPie->SetUnfilledColour(*wxWHITE);	
	
	m_Slider = new wxSlider(this, 10002, 15, 0, 90, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL);
	m_AngleSlider = new wxSlider(this, 10003, 200, 0, 360, wxDefaultPosition, wxDefaultSize);
	wxBoxSizer * sizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer * hsizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer * btnsizer = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(sizer);	
	hsizer->Add(m_ProgressPie, 1, wxEXPAND|wxALL, 5);
	hsizer->Add(m_Pie, 1, wxEXPAND|wxALL, 5);
	hsizer->Add(m_Slider, 0, wxGROW|wxALL, 5);

	wxButton * btn1 = new wxButton(this, 10004, _("Toggle legend transparency"));
	wxButton * btn2 = new wxButton(this, 10005, _("Toggle edges"));
	btnsizer->Add(btn1, 0, wxALL, 5);
	btnsizer->Add(btn2, 0, wxALL, 5);

	sizer->Add(hsizer, 1, wxEXPAND|wxALL, 5);
	sizer->Add(m_AngleSlider, 0, wxGROW|wxALL, 5);
	sizer->Add(btnsizer, 0, wxALIGN_RIGHT|wxBOTTOM|wxLEFT|wxRIGHT, 5);	
	
	sizer->Fit(this);
	m_Timer = new MyTimer(this);
	m_Timer->Start(50);
	SetSize(wxSize(600,350));
	Centre();	
}

MyFrame::~MyFrame()
{
	delete m_Timer;
}

void MyFrame::OnToggleTransparency(wxCommandEvent & event)
{
	m_Pie->GetLegend()->SetTransparent(!m_Pie->GetLegend()->IsTransparent());
	m_Pie->Refresh();
}

void MyFrame::OnToggleEdges(wxCommandEvent & event)
{
	m_Pie->SetShowEdges(!m_Pie->GetShowEdges());
	m_ProgressPie->SetShowEdges(!m_ProgressPie->GetShowEdges());
}

void MyFrame::OnSlider(wxScrollEvent & event)
{
	m_Pie->SetAngle((double)m_Slider->GetValue() / (double)180 * (double)M_PI);
	m_ProgressPie->SetAngle((double)m_Slider->GetValue() / (double)180 * (double)M_PI);
}

void MyFrame::OnAngleSlider(wxScrollEvent & event)
{
	m_Pie->SetRotationAngle((double)m_AngleSlider->GetValue() / (double)180 * (double)M_PI);
	m_ProgressPie->SetRotationAngle((double)m_AngleSlider->GetValue() / (double)180 * (double)M_PI);
}

bool MyApp::OnInit()
{
	wxImage::AddHandler( new wxPNGHandler );
	MyFrame * frame = new MyFrame(NULL);	
	frame->Show();	
	SetTopWindow(frame);
	return true;
}
