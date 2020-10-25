// File:  eBirdCompilerApp.cpp
// Date:  10/25/2020
// Auth:  K. Loux
// Desc:  The application class.

// Local headers
#include "eBirdCompilerApp.h"
#include "mainFrame.h"

IMPLEMENT_APP(EBirdCompilerApp);

const wxString EBirdCompilerApp::title = _T("eBird Compiler");
const wxString EBirdCompilerApp::name = _T("eBirdCompilerApplication");
const wxString EBirdCompilerApp::creator = _T("Kerry Loux");
// gitHash and versionString are defined in gitHash.cpp, which is automatically generated during the build

bool EBirdCompilerApp::OnInit()
{
	SetAppName(name);
	SetVendorName(creator);

	mainFrame = new MainFrame();

	if (!mainFrame)
		return false;

	mainFrame->Show(true);

	return true;
}
