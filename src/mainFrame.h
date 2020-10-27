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
#include <wx/busyinfo.h>

// Standard C++ headers
#include <vector>
#include <memory>
#include <thread>

wxDEFINE_EVENT(THREAD_COMPLETE_EVENT, wxCommandEvent);

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
	void OnThreadCompleteEvent(wxCommandEvent& event);
	
	void UpdateThreadEntry();

	DECLARE_EVENT_TABLE();
	
	EBirdCompiler compiler;
	
	std::thread updateThread;
	std::unique_ptr<wxBusyInfo> busyInfo;
	std::unique_ptr<wxWindowDisabler> windowDisabler;
};

#endif// MAIN_FRAME_H_
