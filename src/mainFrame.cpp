// File:  mainFrame.cpp
// Date:  10/25/2020
// Auth:  K. Loux
// Desc:  Main frame for the application.

// Local headers
#include "mainFrame.h"
#include "eBirdCompilerApp.h"

MainFrame::MainFrame() : wxFrame(nullptr, wxID_ANY, wxEmptyString,
	wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE)
{
	CreateControls();
	SetProperties();
}

void MainFrame::CreateControls()
{
	wxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(this);
	topSizer->Add(panel, wxSizerFlags(1).Expand());
	
	wxSizer *panelSizer = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(panelSizer);
	
	wxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	panelSizer->Add(mainSizer, wxSizerFlags(1).Expand().Border(wxALL, 5));
	
	checklistTextBox = new wxTextCtrl(panel, idChecklistTextChange, wxEmptyString, wxDefaultPosition, wxSize(-1, 150), wxTE_MULTILINE | wxHSCROLL);
	updateButton = new wxButton(panel, idButtonUpdate, _T("Update Summary"));
	updateButton->Enable(false);
	summaryTextBox = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(800, 500), wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH);
	
	wxTextAttr summaryStyle;
	summaryStyle.SetFontFamily(wxFONTFAMILY_MODERN);
	summaryTextBox->SetDefaultStyle(summaryStyle);
	
	mainSizer->Add(new wxStaticText(panel, wxID_ANY, _T("Enter checklist URLs:")), wxSizerFlags().Border(wxALL, 5));
	mainSizer->Add(checklistTextBox, wxSizerFlags().Expand().Border(wxALL, 5));
	mainSizer->Add(updateButton, wxSizerFlags().Border(wxALL, 5).Border(wxALL, 5));
	mainSizer->Add(new wxStaticText(panel, wxID_ANY, _T("Summary of observations:")), wxSizerFlags().Border(wxALL, 5));
	mainSizer->Add(summaryTextBox, wxSizerFlags(1).Expand().Border(wxALL, 5));
	
	SetSizerAndFit(topSizer);
}

void MainFrame::SetProperties()
{
	SetTitle(EBirdCompilerApp::title);
	SetName(EBirdCompilerApp::name);
	Center();

/*#ifdef __WXMSW__
	SetIcon(wxIcon(_T("ICON_ID_MAIN"), wxBITMAP_TYPE_ICO_RESOURCE));
#elif __WXGTK__
	SetIcon(wxIcon(plots16_xpm));
	SetIcon(wxIcon(plots24_xpm));
	SetIcon(wxIcon(plots32_xpm));
	SetIcon(wxIcon(plots48_xpm));
	SetIcon(wxIcon(plots64_xpm));
	SetIcon(wxIcon(plots128_xpm));
#endif*/
}

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
	EVT_BUTTON(idButtonUpdate,			MainFrame::ButtonUpdateClickedEvent)
	EVT_TEXT(idChecklistTextChange,		MainFrame::ChecklistTextChangeEvent)
	EVT_COMMAND(wxID_ANY, THREAD_COMPLETE_EVENT, MainFrame::OnThreadCompleteEvent)
END_EVENT_TABLE();

void MainFrame::UpdateThreadEntry()
{
	wxCommandEvent event(THREAD_COMPLETE_EVENT);
	if (compiler.Update(checklistTextBox->GetValue().ToStdString()))
		event.SetInt(1);
	else
		event.SetInt(0);
		
	wxPostEvent(this, event);
}

void MainFrame::ButtonUpdateClickedEvent(wxCommandEvent& WXUNUSED(event))
{
	windowDisabler = std::make_unique<wxWindowDisabler>();
	busyInfo = std::make_unique<wxBusyInfo>(_T("Gathering checklist data..."));
	
	updateThread = std::thread(&MainFrame::UpdateThreadEntry, this);
	updateButton->Enable(false);
}

void MainFrame::ChecklistTextChangeEvent(wxCommandEvent& WXUNUSED(event))
{
	updateButton->Enable();
}

void MainFrame::OnThreadCompleteEvent(wxCommandEvent& event)
{
	busyInfo.reset();
	windowDisabler.reset();
	if (updateThread.joinable())
		updateThread.join();
	
	if (event.GetInt() == 0)
		wxMessageBox(compiler.GetErrorString(), _T("Error"));
	else
	{
		summaryTextBox->SetValue(compiler.GetSummaryString());
		if (!compiler.GetErrorString().empty())
			wxMessageBox(compiler.GetErrorString(), _T("Warning"));
	}
}
