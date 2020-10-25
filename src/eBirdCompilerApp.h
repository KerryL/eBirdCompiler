// File:  eBirdCompilerApp.h
// Date:  10/25/2020
// Auth:  K. Loux
// Desc:  The application class.

#ifndef EBIRD_COMPILER_APP_H_
#define EBIRD_COMPILER_APP_H_

// wxWidgets headers
#include <wx/wx.h>

// Local forward declarations
class MainFrame;

class EBirdCompilerApp : public wxApp
{
public:
	bool OnInit();

	static const wxString title;// As displayed
	static const wxString name;// Internal
	static const wxString creator;
	static const wxString versionString;
	static const wxString gitHash;

private:
	MainFrame *mainFrame = nullptr;
};

DECLARE_APP(EBirdCompilerApp);

#endif// EBIRD_COMPILER_APP_H_
