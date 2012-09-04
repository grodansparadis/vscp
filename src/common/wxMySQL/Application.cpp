#include "Application.h"
#include "MainFrame.h"

bool wxMySQLTestApp::OnInit()
{
	MainFrame * mainframe = new MainFrame();
	SetTopWindow(mainframe);
	mainframe->Show();
	return true;
}