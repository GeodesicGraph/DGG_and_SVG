#include <wx/wx.h>
#include <wx/panel.h>
#include "test_canvas.h"
class LeftPanel : public wxPanel
{
public:
    LeftPanel(wxPanel *parent);

    void OnLoadModel(wxCommandEvent & event);
    void OnLoadSVG(wxCommandEvent & event);
    void OnLoadDGG(wxCommandEvent & event);
	void OnComputeFD(wxCommandEvent & event);
    void OnComputeSVG(wxCommandEvent & event);
    void OnComputeDGG(wxCommandEvent & event);
		void OnViewingMode(wxCommandEvent & (event));
		void OnSingleSource(wxCommandEvent & (event));
		void OnMultipleSources(wxCommandEvent & (event));

    wxButton *m_plus;
    wxButton *m_minus;
    wxPanel *m_parent;
		wxStaticText*m_text;

		wxTextCtrl* m_text_ctrl_k;
		wxTextCtrl* m_text_ctrl_eps_dgg;
		wxTextCtrl* m_text_thread_num;
		wxTextCtrl* m_text_ctrl_eps_fd;
    int count;

		wxStaticText *m_mesh_text_message;
		wxStaticText *m_graph_text_message;


};

class RightPanel : public wxPanel
{
public:
    RightPanel(wxPanel *parent);

    //void OnSetText(wxCommandEvent & event);
		///void TestGLCanvas::OnPaint(wxCommandEvent & event);
   // wxStaticText *m_text;
		TestGLCanvas *m_canvas;

};

const int ID_Load_model = 101;
const int ID_Load_SVG = 102;
const int ID_Compute_SVG = 103;
const int ID_Load_DGG = 104;
const int ID_Compute_DGG = 105;
const int ID_Compute_FastDGG = 109;
const int ID_Viewing_Mode = 106;
const int ID_Single_Source = 107;
const int ID_Multiple_Sources = 108;
