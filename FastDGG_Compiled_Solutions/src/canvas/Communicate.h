#include "Panels.h"
#include "test_canvas.h"
#include <wx/wxprec.h>


class Communicate : public wxFrame
{
public:
    Communicate(const wxString& title);


    LeftPanel *m_lp;
    RightPanel *m_rp;
	//  void SetCanvas(TestGLCanvas *canvas) { m_canvas = canvas; }
    //TestGLCanvas *GetCanvas() { return m_canvas; }

    wxPanel *m_parent;

};