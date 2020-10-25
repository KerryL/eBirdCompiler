// File:  mainFrame.h
// Date:  10/25/2020
// Auth:  K. Loux
// Desc:  Main frame for the application.

#ifndef MAIN_FRAME_H_
#define MAIN_FRAME_H_

// Local headers
#include "eBirdCompiler.h"

// wxWidgets headers
#include <wx/wx.h>

// Standard C++ headers
#include <vector>

// The main frame class
class MainFrame : public wxFrame
{
public:
	MainFrame();
	~MainFrame() = default;

private:
	void CreateControls();
	void SetProperties();
	
	wxTextCtrl* checklistTextBox;
	wxTextCtrl* summaryTextBox;
	
	wxButton* updateButton;

	// The event IDs
	enum MainFrameEventID
	{
		idButtonUpdate = wxID_HIGHEST + 100,
		idChecklistTextChange
	};

	void ButtonUpdateClickedEvent(wxCommandEvent &event);
	void ChecklistTextChangeEvent(wxCommandEvent& event);

	DECLARE_EVENT_TABLE();
	
	EBirdCompiler compiler;
};

#endif// MAIN_FRAME_H_
