#include <wx/stattext.h>
#include "Communicate.h"

LeftPanel::LeftPanel(wxPanel * parent)
	: wxPanel(parent, -1, wxPoint(-1, -1), wxSize(-1, -1), wxBORDER_SUNKEN)
{
	count = 0;
	m_parent = parent;

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	{
		wxStaticBoxSizer *model_loader_box = new wxStaticBoxSizer(wxVERTICAL, this, "Model");

			wxBoxSizer  *loader_box = new wxBoxSizer(wxHORIZONTAL);
			loader_box->Add(new wxButton(this, ID_Load_model, wxT("Load")), 0, wxLEFT, 0);
			loader_box->Add(20, 10);
			auto thread_text = new wxStaticText(this, -1, "Threads");
			loader_box->Add(thread_text, 0, wxLEFT, 10);
			m_text_thread_num = new wxTextCtrl(this, wxID_ANY, "4", wxPoint(-1, -1), wxSize(40, 25));
			loader_box->Add(m_text_thread_num, 0, wxLEFT, 10);

		model_loader_box->Add(loader_box);

		vbox->Add(model_loader_box, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
		Connect(ID_Load_model, wxEVT_COMMAND_BUTTON_CLICKED,
			wxCommandEventHandler(LeftPanel::OnLoadModel));
	}
	{

		wxStaticBoxSizer *dgg_loader_box = new wxStaticBoxSizer(wxVERTICAL, this, "Fast DGG");


		//svg_loader_box->Add( new wxButton(this, ID_PLUS, wxT("Load"), wxPoint(10, 10)));

		wxBoxSizer  *dgg_eps_box = new wxBoxSizer(wxHORIZONTAL);
		auto eps_text = new wxStaticText(this, -1, "\u03B5");
		dgg_eps_box->Add(eps_text, 0, wxLEFT, 10);
		m_text_ctrl_eps_fd = new wxTextCtrl(this, wxID_ANY, "0.001", wxPoint(-1, -1), wxSize(40, 25));
		dgg_eps_box->Add(m_text_ctrl_eps_fd, 0, wxLEFT, 10);
		
		dgg_eps_box->Add(new wxButton(this, ID_Compute_FastDGG, wxT("Compute"), wxPoint(-1, -1), wxSize(80, 25)), 0, wxLEFT, 8);
		Connect(ID_Compute_FastDGG, wxEVT_COMMAND_BUTTON_CLICKED,
			wxCommandEventHandler(LeftPanel::OnComputeFD));
		dgg_eps_box->Add(new wxButton(this, ID_Load_DGG, wxT("Load"), wxPoint(-1, -1), wxSize(60, 25)), 0, wxLEFT, 4);
		//dgg_loader_box->Add(-1, 10);
		Connect(ID_Load_DGG, wxEVT_COMMAND_BUTTON_CLICKED,
			wxCommandEventHandler(LeftPanel::OnLoadDGG));

		dgg_loader_box->Add(dgg_eps_box);

		vbox->Add(dgg_loader_box, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
	}

	{

		wxStaticBoxSizer *dgg_loader_box = new wxStaticBoxSizer(wxVERTICAL, this, "DGG");

		wxBoxSizer  *dgg_eps_box = new wxBoxSizer(wxHORIZONTAL);
		auto eps_text = new wxStaticText(this, -1, "\u03B5");
		dgg_eps_box->Add(eps_text, 0, wxLEFT, 10);
		m_text_ctrl_eps_dgg = new wxTextCtrl(this, wxID_ANY, "0.001", wxPoint(-1, -1), wxSize(40, 25));
		dgg_eps_box->Add(m_text_ctrl_eps_dgg, 0, wxLEFT, 10);

		dgg_eps_box->Add(new wxButton(this, ID_Compute_DGG, wxT("Compute"), wxPoint(-1, -1), wxSize(80, 25)), 0, wxLEFT, 8);
		Connect(ID_Compute_DGG, wxEVT_COMMAND_BUTTON_CLICKED,
			wxCommandEventHandler(LeftPanel::OnComputeDGG));
		dgg_eps_box->Add(new wxButton(this, ID_Load_DGG, wxT("Load"), wxPoint(-1, -1), wxSize(60, 25)), 0, wxLEFT, 4);
		Connect(ID_Load_DGG, wxEVT_COMMAND_BUTTON_CLICKED,
			wxCommandEventHandler(LeftPanel::OnLoadDGG));

		dgg_loader_box->Add(dgg_eps_box);

		vbox->Add(dgg_loader_box, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
	}

		//{

		//	wxStaticBoxSizer *dgg_loader_box = new wxStaticBoxSizer(wxVERTICAL, this, "DGG");

		//	wxBoxSizer  *thread_box = new wxBoxSizer(wxHORIZONTAL);
		//	auto eps_text = new wxStaticText(this, -1, "\u03B5");
		//	thread_box->Add(eps_text, 0, wxLEFT, 10);
		//	m_text_ctrl_eps_dgg = new wxTextCtrl(this, wxID_ANY, "0.001", wxPoint(-1, -1), wxSize(40, 25));
		//	thread_box->Add(m_text_ctrl_eps_dgg, 0, wxLEFT, 10);

		//	wxBoxSizer  *dgg_eps_box = new wxBoxSizer(wxHORIZONTAL);
		//	dgg_eps_box->Add(new wxButton(this, ID_Load_DGG, wxT("Load")), 0, wxALIGN_RIGHT, 0);
		//	Connect(ID_Load_DGG, wxEVT_COMMAND_BUTTON_CLICKED,
		//		wxCommandEventHandler(LeftPanel::OnLoadDGG));
		//	dgg_eps_box->Add(new wxButton(this, ID_Compute_DGG, wxT("Compute")), 0, wxLEFT, 8);
		//	Connect(ID_Compute_DGG, wxEVT_COMMAND_BUTTON_CLICKED,
		//		wxCommandEventHandler(LeftPanel::OnComputeDGG));

		//	dgg_loader_box->Add(thread_box);
		//	dgg_loader_box->Add(dgg_eps_box);

		//	vbox->Add(dgg_loader_box, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
		//}

	{
		wxStaticBoxSizer *svg_loader_box = new wxStaticBoxSizer(wxVERTICAL,this,"SVG");


		//svg_loader_box->Add( new wxButton(this, ID_PLUS, wxT("Load"), wxPoint(10, 10)));
		wxBoxSizer  *svg_k_box = new wxBoxSizer(wxHORIZONTAL);
		svg_k_box->Add(new wxStaticText(this,-1,"K"),0,wxLEFT,10);
		m_text_ctrl_k = new wxTextCtrl(this, wxID_ANY,"50",wxPoint(-1,-1),wxSize(40,25));
		svg_k_box->Add(m_text_ctrl_k,0,wxLEFT,10);

		svg_k_box->Add(new wxButton(this, ID_Compute_SVG, wxT("Compute"), wxPoint(-1, -1), wxSize(80, 25)), 0, wxLEFT, 8);
		Connect(ID_Compute_SVG, wxEVT_COMMAND_BUTTON_CLICKED, 
			wxCommandEventHandler(LeftPanel::OnComputeSVG));
		svg_k_box->Add(new wxButton(this, ID_Load_SVG, wxT("Load"), wxPoint(-1, -1), wxSize(60, 25)), 0, wxLEFT, 4);
		Connect(ID_Load_SVG, wxEVT_COMMAND_BUTTON_CLICKED,
			wxCommandEventHandler(LeftPanel::OnLoadSVG));

		svg_loader_box->Add(svg_k_box);
		vbox->Add(svg_loader_box, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
	}

	{

		wxStaticBoxSizer *geodesic_box = new wxStaticBoxSizer(wxVERTICAL,this,"Geodesic Distances");

		wxBoxSizer  *dgg_loader_box1 = new wxBoxSizer(wxHORIZONTAL);
		dgg_loader_box1->Add( new wxButton(this, ID_Viewing_Mode, wxT("Viewing Mode"),wxPoint(-1,-1),wxSize(214,25)), 0, wxALIGN_RIGHT, 0);
		Connect(ID_Viewing_Mode, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LeftPanel::OnViewingMode));


		wxBoxSizer  *dgg_loader_box = new wxBoxSizer(wxHORIZONTAL);
		dgg_loader_box->Add( new wxButton(this, ID_Single_Source, wxT("Single-Source"),wxPoint(-1,-1),wxSize(105,25)), 0, wxLEFT, 0);
		Connect(ID_Single_Source, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LeftPanel::OnSingleSource));
		dgg_loader_box->Add(-1, 10);
		dgg_loader_box->Add(new wxButton(this, ID_Multiple_Sources, wxT("Multiple-Source"), wxPoint(-1, -1), wxSize(105, 25)), 0, wxLEFT, 4);
		dgg_loader_box->Add(-1, 10);
		Connect(ID_Multiple_Sources, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LeftPanel::OnMultipleSources));

		geodesic_box->Add(dgg_loader_box1);
		geodesic_box->Add(dgg_loader_box);

		vbox->Add(geodesic_box, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
	}
#if 0
	{
		m_text_message = new wxStaticText(this,-1,"");
		vbox->Add(m_text_message, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);

	}
#else
	{
		wxBoxSizer *mesh_info_box = new wxStaticBoxSizer(wxVERTICAL,this,"Mesh");
		m_mesh_text_message = new wxStaticText(this,-1,
			"#Vertices N.A.\n#Edges N.A.\n#Faces N.A.\n"
		);
		//m_mesh_text_message = new wxStaticText(this, -1,
		//	"#Vertices N.A.\n#Faces N.A.\n"
		//	);
		mesh_info_box->Add(m_mesh_text_message, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
		vbox->Add(mesh_info_box, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
	}

	{
		wxBoxSizer *graph_info_box = new wxStaticBoxSizer(wxVERTICAL,this,"Graph");
		//m_graph_text_message = new wxStaticText(this,-1,
		//	("#Vertices N.A.\n#Edges N.A.\nAvg. Degree = N.A.\n¦Å = N.A.\nTime for computing candidate edges\n   N.A. seconds\nTime for edge pruning\n   N.A. seconds\n")
		//);
		m_graph_text_message = new wxStaticText(this, -1,
			("#Vertices N.A.\n#Edges N.A.\nAvg. Degree = N.A.\n\u03B5 = N.A.\nTime for computing candidate edges\n   N.A. seconds\n")
			);
		graph_info_box->Add(m_graph_text_message, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
		vbox->Add(graph_info_box, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
	}
#endif
	SetSizer(vbox);
}

void LeftPanel::OnLoadModel(wxCommandEvent & WXUNUSED(event))
{
	Communicate *comm = (Communicate *) m_parent->GetParent();
	//wxString message;
	string message;
	comm->m_rp->m_canvas->viewer_.LoadModel(message);
	wxLogMessage(message.c_str());
	m_mesh_text_message->SetLabel(message);
	comm->m_rp->m_canvas->Refresh(true);
	

}

void LeftPanel::OnLoadSVG(wxCommandEvent & WXUNUSED(event))
{
	Communicate *comm = (Communicate *) m_parent->GetParent();
	int k ;
	string message;
	comm->m_rp->m_canvas->viewer_.LoadSVG(k,message);
	TCHAR buf[100];
	swprintf(buf,_T("%d"),k);
	m_text_ctrl_k->SetValue(buf);
	m_graph_text_message->SetLabel(message);
	comm->m_rp->m_canvas->Refresh(true);
}

void LeftPanel::OnLoadDGG(wxCommandEvent & WXUNUSED(event))
{
	Communicate *comm = (Communicate *) m_parent->GetParent();
	double eps;
	string message;
	comm->m_rp->m_canvas->viewer_.LoadDGG(eps,message);
	//TCHAR buf[100];
	//swprintf(buf,_T("%lf"),eps);
	//m_text_ctrl_eps->SetValue(buf);
	m_graph_text_message->SetLabel(message);
	comm->m_rp->m_canvas->Refresh(true);
}



void LeftPanel::OnComputeSVG(wxCommandEvent & WXUNUSED(event))
{
	Communicate *comm = (Communicate *) m_parent->GetParent();
	wxString k_wxstring = m_text_ctrl_k->GetValue();
	//(string)k_wxstring.ToAscii();
	int k = atoi(k_wxstring.ToAscii());
	wxString thread_num_wxString = m_text_thread_num->GetValue();
	int thread_num = atoi(thread_num_wxString.ToAscii());
	string message;
	comm->m_rp->m_canvas->viewer_.ComputeSVG(k, thread_num, message);
	m_graph_text_message->SetLabel(message);
	comm->m_rp->m_canvas->Refresh(true);
}

void LeftPanel::OnComputeFD(wxCommandEvent & WXUNUSED(event))
{
	Communicate *comm = (Communicate *)m_parent->GetParent();
	wxString eps_wxstring = m_text_ctrl_eps_fd->GetValue();
	double eps = atof(eps_wxstring.ToAscii());
	wxString thread_num_wxString = m_text_thread_num->GetValue();
	int thread_num = atoi(thread_num_wxString.ToAscii());
	string message;
	comm->m_rp->m_canvas->viewer_.ComputeFD(eps, thread_num, message);
	TCHAR buf[100];
	swprintf(buf, _T("%lf"), eps);
	m_text_ctrl_eps_fd->SetValue(buf);
	m_graph_text_message->SetLabel(message);
	comm->m_rp->m_canvas->Refresh(true);
}

void LeftPanel::OnComputeDGG(wxCommandEvent & WXUNUSED(event))
{
	Communicate *comm = (Communicate *) m_parent->GetParent();
	wxString eps_wxstring = m_text_ctrl_eps_dgg->GetValue();
	double eps = atof(eps_wxstring.ToAscii());
	wxString thread_num_wxString = m_text_thread_num->GetValue();
	int thread_num = atoi(thread_num_wxString.ToAscii());
	string message;
	comm->m_rp->m_canvas->viewer_.ComputeDGG(eps, thread_num, message);
	TCHAR buf[100];
	swprintf(buf,_T("%lf"),eps);
	m_text_ctrl_eps_dgg->SetValue(buf);
	m_graph_text_message->SetLabel(message);
	comm->m_rp->m_canvas->Refresh(true);
}

void LeftPanel::OnViewingMode(wxCommandEvent & WXUNUSED(event))
{
	Communicate *comm = (Communicate *) m_parent->GetParent();
	comm->m_rp->m_canvas->viewer_.toViewingModel();
	comm->m_rp->m_canvas->Refresh(true);
}

void LeftPanel::OnSingleSource(wxCommandEvent & WXUNUSED(event))
{
	Communicate *comm = (Communicate *) m_parent->GetParent();
	comm->m_rp->m_canvas->viewer_.toSingleSource();
	comm->m_rp->m_canvas->Refresh(true);
}
void LeftPanel::OnMultipleSources(wxCommandEvent & WXUNUSED(event))
{
	Communicate *comm = (Communicate *) m_parent->GetParent();
	comm->m_rp->m_canvas->viewer_.toMultipleSources();
	comm->m_rp->m_canvas->Refresh(true);
}
//void LeftPanel::OnMinus(wxCommandEvent & WXUNUSED(event))
//{
//  count--;
//
//  Communicate *comm = (Communicate *) m_parent->GetParent();
//  //comm->m_rp->m_text->SetLabel(wxString::Format(wxT("%d"), count));
//}


RightPanel::RightPanel(wxPanel * parent)
       : wxPanel(parent, wxID_ANY, wxDefaultPosition, 
         wxSize(800, 800), wxBORDER_SUNKEN)
{
	// m_text = new wxStaticText(this, -1, wxT("0"), wxPoint(40, 60));
	m_canvas = new TestGLCanvas(this, wxID_ANY, wxDefaultPosition,
	GetClientSize(), wxSUNKEN_BORDER);

	m_canvas->SetCurrent();
	m_canvas->Show( true ); 
}