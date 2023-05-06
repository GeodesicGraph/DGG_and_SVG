#include "Communicate.h"
#include "test_canvas.h"

Communicate::Communicate(const wxString& title)
       : wxFrame(NULL, wxID_ANY, title, wxPoint(0, 0), wxSize(1000, 800))
{
  m_parent = new wxPanel(this, wxID_ANY);

  wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);

  m_lp = new LeftPanel(m_parent);
  m_rp = new RightPanel(m_parent);

  hbox->Add(m_lp, 1, wxEXPAND | wxALL, 1);
  hbox->Add(m_rp, 4, wxEXPAND | wxALL, 1);

  m_parent->SetSizer(hbox);


  //this->Centre();
}