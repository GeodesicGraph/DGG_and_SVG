/////////////////////////////////////////////////////////////////////////////
// Name:        penguin.cpp
// Purpose:     wxGLCanvas demo program
// Author:      Robert Roebling
// Modified by: Sandro Sigala
// Created:     04/01/98
// Copyright:   (c) Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if !wxUSE_GLCANVAS
    #error "OpenGL required: set wxUSE_GLCANVAS to 1 and rebuild the library"
#endif

#include "test_canvas.h"
#ifdef __DARWIN__
    #include <OpenGL/glu.h>
#else
    #include <GL/glu.h>
#endif

#include "sample.xpm"
#include "ICH/RichModel.h"
#include "Communicate.h"

#include <string>
using std::string;


// ---------------------------------------------------------------------------
// MyApp
// ---------------------------------------------------------------------------

// `Main program' equivalent, creating windows and returning main app frame


// ---------------------------------------------------------------------------
// T  estGLCanvas
// ---------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(TestGLCanvas, wxGLCanvas)
    EVT_SIZE(TestGLCanvas::OnSize)
    EVT_PAINT(TestGLCanvas::OnPaint)
    EVT_ERASE_BACKGROUND(TestGLCanvas::OnEraseBackground)
    EVT_MOUSE_EVENTS(TestGLCanvas::OnMouse)
wxEND_EVENT_TABLE()

TestGLCanvas::TestGLCanvas(wxWindow *parent,
                           wxWindowID id,
                           const wxPoint& pos,
                           const wxSize& size,
                           long style,
                           const wxString& name)
    : wxGLCanvas(parent, id, NULL, pos, size,
                 style | wxFULL_REPAINT_ON_RESIZE, name)
{
    // Explicitly create a new rendering context instance for this canvas.
    m_glRC = new wxGLContext(this);

    m_gldata.initialized = false;

    // initialize view matrix
    m_gldata.beginx = 0.0f;
    m_gldata.beginy = 0.0f;
    m_gldata.zoom   = 45.0f;
    trackball(m_gldata.quat, 0.0f, 0.0f, 0.0f, 0.0f);

		viewer_.initial_mesh();


}

TestGLCanvas::~TestGLCanvas()
{
    delete m_glRC;
}

void TestGLCanvas::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
	//wxLog::SetActiveTarget(new wxLogStderr()) ;


    // must always be here
    wxPaintDC dc(this);

    SetCurrent(*m_glRC);

    // Initialize OpenGL
    if (!m_gldata.initialized)
    {
        InitGL();
        ResetProjectionMode();

        m_gldata.initialized = true;
    }

		viewer_.display();
		if(false) {
			// Clear
			glClearColor( 0.3f, 0.4f, 0.6f, 1.0f );
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

			// Transformations
			glLoadIdentity();
			glTranslatef( 0.0f, 0.0f, -20.0f );
			GLfloat m[4][4];
			build_rotmatrix( m, m_gldata.quat );
			glMultMatrixf( &m[0][0] );
			m_renderer.Render();
		}
	//	viewer_.g_model_ptr->Render(GL_RENDER);

					// Flush
			glFlush();

			// Swap
			SwapBuffers();
}

void TestGLCanvas::OnSize(wxSizeEvent& WXUNUSED(event))
{
    // Reset the OpenGL view aspect.
    // This is OK only because there is only one canvas that uses the context.
    // See the cube sample for that case that multiple canvases are made current with one context.
    //ResetProjectionMode();
		  // set viewport and frustum, and update camera
      int w, h;
    GetClientSize(&w, &h);

		viewer_.reshape(w,h);
}

void TestGLCanvas::OnEraseBackground(wxEraseEvent& WXUNUSED(event))
{
    // Do nothing, to avoid flashing on MSW
}

// Load the DXF file.  If the zlib support is compiled in wxWidgets,
// supports also the ".dxf.gz" gzip compressed files.
void TestGLCanvas::LoadDXF(const wxString& filename)
{
    wxFileInputStream stream(filename);
    if (stream.IsOk())
#if wxUSE_ZLIB
    {
        if (filename.Right(3).Lower() == wxT(".gz"))
        {
            wxZlibInputStream zstream(stream);
            m_renderer.Load(zstream);
        } else
        {
            m_renderer.Load(stream);
        }
    }
#else
    {
        m_renderer.Load(stream);
    }
#endif
}

void TestGLCanvas::OnMouse(wxMouseEvent& event)
{
	if (event.Dragging())
	{
		viewer_.rotateCamera(event.GetX(),event.GetY());
		viewer_.continuousPickSingleSource(event.GetX(),event.GetY());
		/* orientation has changed, redraw mesh */
		Refresh(false);
	}else if(event.LeftDown()) {
		Communicate *comm = (Communicate *)(this->GetParent()->GetParent());
		string message;
		viewer_.mouseLeftDown( event.GetX(), event.GetY(),message);
		//comm->m_lp->m_text_message->SetLabel(message);
		//viewer_.g_vLastMousePos = Wml::Vector2f(event.GetX(),event.GetY());
		/* orientation has changed, redraw mesh */
		Refresh(false);
	}
    m_gldata.beginx = event.GetX();
    m_gldata.beginy = event.GetY();
}

void TestGLCanvas::InitGL()
{

	if(false) {
		static const GLfloat light0_pos[4]   = { -50.0f, 50.0f, 0.0f, 0.0f };

		// white light
		static const GLfloat light0_color[4] = { 0.6f, 0.6f, 0.6f, 1.0f };

		static const GLfloat light1_pos[4]   = {  50.0f, 50.0f, 0.0f, 0.0f };

		// cold blue light
		static const GLfloat light1_color[4] = { 0.4f, 0.4f, 1.0f, 1.0f };

		/* remove back faces */
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);

		/* speedups */
		glEnable(GL_DITHER);
		glShadeModel(GL_SMOOTH);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
		glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);

		/* light */
		glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
		glLightfv(GL_LIGHT0, GL_DIFFUSE,  light0_color);
		glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
		glLightfv(GL_LIGHT1, GL_DIFFUSE,  light1_color);
		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHT1);
		glEnable(GL_LIGHTING);

		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		glEnable(GL_COLOR_MATERIAL);
	}

	viewer_.setupGLstate();
}

void TestGLCanvas::ResetProjectionMode()
{
	if (!IsShownOnScreen())
		return;

	// This is normally only necessary if there is more than one wxGLCanvas
	// or more than one wxGLContext in the application.
	SetCurrent(*m_glRC);

	int w, h;
	GetClientSize(&w, &h);
	// It's up to the application code to update the OpenGL viewport settings.
	// In order to avoid extensive context switching, consider doing this in
	// OnPaint() rather than here, though.
	glViewport(0, 0, (GLint) w, (GLint) h);
	viewer_.reshape(w, h);

	if(false) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(45.0f, (GLfloat)w/h, 1.0, 100.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
}

