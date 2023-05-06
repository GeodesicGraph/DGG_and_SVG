

#include "stdafx.h"
#include "Shlwapi.h"
#include "RgbImage.h"
#include <string>
#include <locale>
#include <codecvt>
#include "RMS_LIB/MeshObject.h"
#include "RMS_LIB/Frame.h"
#include "RMS_LIB/ExtendedWmlCamera.h"
#include "ICH/RichModel.h"

#include "dgg_dijkstra.h"
#include "dgg_time.h"
#include "dgg_util.h"

#include "wx/wx.h"

using namespace std;


class Viewer{

	MeshObject g_mesh;
	GLuint texture;
	Wml::ExtendedWmlCamera * g_pCamera;
	SparseGraph<float>* g_svg_graph;

	enum MethodMode{
		METHOD_DISTANCE_FIELD
	};

	MethodMode g_method_mode;

	enum DISTANCE_FIELD_SELECT_POINT_STATUS{
		DISTANCE_FIELD_UNINITIAL, DISTANCE_FIELD_COMPUTED, DISTANCE_FIELD_CHANGED
	} g_distance_field_status;

	enum MouseClickMode{
		NONE,
		SELECT_POINT,
		ADD_POINT
	};
	MouseClickMode mouse_click_mode;
	CPoint3D hit_point;
	int g_hit_vertex_id;
	vector<int> g_hit_vertices;
	enum GRAPH_TYPE{
		GRAPH_DGG, GRAPH_SVG
	};
	int g_graph_type;// = GRAPH_SVG;//0 for svg , 1 for dgg
	int g_svg_k;
	double g_eps_vg;
	string g_exe_dir;

public:
	Wml::Vector2f g_vLastMousePos;
	CRichModel* g_model_ptr;



public:
	Viewer() {
		g_pCamera = NULL;
		g_method_mode =  METHOD_DISTANCE_FIELD;
		g_distance_field_status = DISTANCE_FIELD_UNINITIAL;
		mouse_click_mode = NONE;
		g_svg_graph = NULL;
		hit_point = CPoint3D(0,0,0);
		g_hit_vertex_id = -1;
		g_hit_vertices.push_back(1);
		g_vLastMousePos = Wml::Vector2f::ZERO;
		g_graph_type = GRAPH_SVG;//0 for svg , 1 for dgg
		g_svg_k = 50;
		g_model_ptr = NULL;
		char cCurrentPath[FILENAME_MAX];
		_getcwd(cCurrentPath, sizeof(cCurrentPath));
		g_exe_dir = cCurrentPath;	
		g_eps_vg = 0.01;
	}

	string getFileNameFromPath(const string& path)
	{
		char drive[MAX_PATH];
		char dir[MAX_PATH];
		char fname[MAX_PATH];
		char ext[MAX_PATH];
		_splitpath(path.c_str(), drive, dir, fname, ext);
		return (string)fname;
	}

	void initial_mesh()
	{
		//string obj_file_name = "bunny_nf10k.obj";

		//string texture_distance_file_name = "data\\pic\\texture_distance_map.bmp";

		//g_model_ptr = new CRichModel(obj_file_name);
		//g_model_ptr->Preprocess();

		//g_mesh.ReadCRichModel(*g_model_ptr);
		//readTexture(texture_distance_file_name);

		wxLog::SetActiveTarget(new wxLogStderr()) ;
		wxLogMessage("asldfkjasl;kdfjasf");



	}

	void rotateCamera(int x, int y) {
		if ( mouse_click_mode != NONE) return;
		static const float fPanScale = 0.01f;
		static const float fZoomScale = 0.01f;
		static const float fOrbitScale = 0.01f;
		static const float fExpMapScale = 0.005f;
		Wml::Vector2f vCur(x,y);
		Wml::Vector2f vDelta(vCur - g_vLastMousePos);
		g_vLastMousePos = vCur;
		g_pCamera->OrbitLateral(-vDelta.X() * fOrbitScale);
		g_pCamera->OrbitVertical(vDelta.Y() * fOrbitScale);
	}




	void reshape(int w, int h)
	{
		// set viewport and frustum, and update camera
		g_pCamera->SetViewPort(0, 0, w, h);
		g_pCamera->SetFrustum( 40.0f, float(w)/float(h), 0.01f, 100.0);
		g_pCamera->Update();
		// Send the new window size to AntTweakBar
	}

	void readTexture(string filename)
	{
		RgbImage theTexMap( filename.c_str() );

		// Pixel alignment: each row is word aligned (aligned to a 4 byte boundary)
		//    Therefore, no need to call glPixelStore( GL_UNPACK_ALIGNMENT, ... );

		// allocate a texture name
		glGenTextures( 1, &texture );
		glBindTexture( GL_TEXTURE_2D, texture );
		// select modulate to mix texture with color for shading
		glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

		// when texture area is small, bilinear filter the closest mipmap
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_NEAREST );
		// when texture area is large, bilinear filter the first mipmap
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		// if wrap is true, the texture wraps over at the edges (repeat)
		//       ... false, the texture ends at the edges (clamp)
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
			GL_CLAMP );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
			GL_CLAMP );
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3,theTexMap.GetNumCols(), theTexMap.GetNumRows(),
			GL_RGB, GL_UNSIGNED_BYTE, theTexMap.ImageData() );

		//gluBuild2DMipmaps(GL_TEXTURE_2D,3,ImageWidth,ImageHeight,GL_RGB,GL_UNSIGNED_BYTE,&Image[0][0][0]);
		//glEnable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_2D);
	}



	void setupGLstate() {

		g_pCamera = new Wml::ExtendedWmlCamera(1.0f,1.0f);
		g_pCamera->SetTargetFrame( Wml::Vector3f(0.0, 0.0, 4.0f), 
			Wml::Vector3f(-1.0f, 0.0, 0.0),
			Wml::Vector3f(0.0, 1.0f, 0.0), 
			Wml::Vector3f(0.0, 0.0, 0.0));


		GLfloat lightOneColor[] = {0.6, 0.6, 0.6, 1};
		GLfloat globalAmb[] = {.1, .1, .1, 1};
		//GLfloat lightOnePosition[] = {.0,  .2, 1, 0.0};
		GLfloat lightOnePosition[] = {.0,  .0, 1, 0.0};

		glEnable(GL_CULL_FACE);
		//glFrontFace(GL_CCW);      
		glEnable(GL_DEPTH_TEST);
		//glClearColor(0,0,0,0);
		glShadeModel(GL_SMOOTH);


		glEnable(GL_LIGHT1);
		glEnable(GL_LIGHTING);
		glEnable(GL_NORMALIZE);
		glEnable(GL_COLOR_MATERIAL);

		glLightfv(GL_LIGHT1, GL_DIFFUSE, lightOneColor);
		//glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmb);
		glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

		glLightfv(GL_LIGHT1, GL_POSITION, lightOnePosition);

		glEnable (GL_LINE_SMOOTH);
		glHint (GL_LINE_SMOOTH, GL_NICEST);

	}


	void display()
	{
		const vector<float>* distance_field = NULL;
		const vector<CPoint3D>* sampled_points = NULL;
		vector<float>* sampled_distance_field = NULL;
		vector<CPoint3D>* tangent_field_ptr = NULL;

		if (g_method_mode == METHOD_DISTANCE_FIELD) {
			if (g_distance_field_status == DISTANCE_FIELD_CHANGED) {
				if (mouse_click_mode == SELECT_POINT) {
					if (g_svg_graph != NULL) {
						//printf("distance_field computing");
						ElapasedTime t;
						g_svg_graph->findShortestDistance(g_hit_vertex_id, true);
						t.printTime("t");
						//printf("distance_field computed\n");
						distance_field = g_svg_graph->distanceField();    
						char buf_message1[1024];
						sprintf(buf_message1,"Select vertex %d" , g_hit_vertex_id);
						char buf_message2[1024];
						sprintf(buf_message2,"%lf seconds for geodesic distance field ", t.getTime());
						//printButtonMessage(buf_message1,buf_message2);

						g_distance_field_status = DISTANCE_FIELD_COMPUTED;
					}
				} else if (mouse_click_mode == ADD_POINT) {
					if (g_svg_graph != NULL) {
						ElapasedTime t;
						g_svg_graph->findShortestDistance(g_hit_vertices,true);
						distance_field = g_svg_graph->distanceField();                       
						g_distance_field_status = DISTANCE_FIELD_COMPUTED;
						char buf_message1[1024];
						sprintf(buf_message1,"Add vertex %d" , g_hit_vertices.back());
						char buf_message2[1024];
						sprintf(buf_message2,"%lf seconds for geodesic distance field ", t.getTime());
						//printButtonMessage(buf_message1,buf_message2);
					}
				}
			} else if (g_distance_field_status == DISTANCE_FIELD_COMPUTED) {
				//if (g_hit_vertex_id >= 0) {
				if (g_svg_graph != NULL) {
					distance_field = g_svg_graph->distanceField();
				}
				//}
			}
		} 
		//	return;
		/* clear frame buffer */
		//glClearColor(30.0/255.0,144.0/255.0,255.0/255.0, 0.0f); 
		glClearColor(220.0/255.0,234.0/255.0, 248.0/255.0,1.0f);
		//glClearColor(234.0 / 255.0 , 234.0 / 255.0 , 234.0 / 255.0 , 1.0f);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//setupLight();

		//setupEye();
		/* transform from the eye coordinate system to the world system */
		//glPushMatrix();
		/* transform from the world to the ojbect coordinate system */
		//setupObject();
		/* draw the mesh */
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		//glTranslatef( 0.0f, 0.0f, -20.0f );

		g_pCamera->Update();

		wxLogMessage("display");

		int obj_shade_mode = 0;
		if (g_model_ptr != NULL) {
			color_white.SetColor();
			if (obj_shade_mode == 0) {
				if (g_method_mode ==METHOD_DISTANCE_FIELD) {
					if (g_distance_field_status == DISTANCE_FIELD_COMPUTED) {
						wxLogMessage("display texture");
						glEnable(GL_TEXTURE_2D);

						g_model_ptr->Render(GL_RENDER, distance_field);
						glDisable(GL_TEXTURE_2D);
					} else {
						g_model_ptr->Render(GL_RENDER);
						wxLogMessage("g_model_ptr");
					}
				} 
			}else if(obj_shade_mode == 1) {
				glShadeModel(GL_FLAT);
				g_model_ptr->Render(GL_RENDER);
			}else if(obj_shade_mode == 2) {
				glShadeModel(GL_SMOOTH);
				g_model_ptr->Render(GL_RENDER);
				g_model_ptr->DrawWireframe();
			}else if(obj_shade_mode == 3) {
				g_model_ptr->DrawPoint();
			}
		}
		//	wxLogMessage("display???");

	}


	string toRelativePath(const string& input_filename) {
		TCHAR buf[MAX_PATH*2];
		std::wstring wsTmp(input_filename.begin(), input_filename.end());

		PathRelativePathTo(buf,
			s2ws(ExePath()).c_str(),
			FILE_ATTRIBUTE_DIRECTORY,
			wsTmp.c_str(),
			FILE_ATTRIBUTE_NORMAL);

		string result = ws2s(buf);
		if (result[0] == '.' && result[1] == '\\') {
			return result.substr(2);
		}else{
			return result;
		}
	}

	string toFullPath(const string& input_filename)
	{
		//TCHAR buffer[MAX_PATH];
		//GetCurrentDirectory(MAX_PATH,buffer);
		//char buffer_c[MAX_PATH];
		//wcstombs(buffer_c, buffer, wcslen(buffer));
		return ExePath() + (string)"\\" + input_filename;
	}

	string ExePath() {
		return g_exe_dir;
	}

	wstring s2ws(const std::string& str)
	{
		typedef std::codecvt_utf8<wchar_t> convert_typeX;
		std::wstring_convert<convert_typeX, wchar_t> converterX;

		return converterX.from_bytes(str);
	}

	string ws2s(const std::wstring& wstr)
	{
		typedef std::codecvt_utf8<wchar_t> convert_typeX;
		std::wstring_convert<convert_typeX, wchar_t> converterX;

		return converterX.to_bytes(wstr);
	}

	void LoadModel(string& message)
	{

		OPENFILENAME ofn;
		TCHAR szFile[MAX_PATH]  = _T("\0");
		ZeroMemory(&ofn , sizeof(ofn));
		ofn.lStructSize			= sizeof(ofn);
		//ofn.hwndOwner			=  FindWindow(NULL, "Mesh-Viewer");	//find main window to freeze it
		ofn.lpstrFile			= szFile ;
		//ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile			= sizeof(szFile);
		ofn.lpstrFilter			= _T("Model file (.m, .obj)\0*.m;*.obj\0All\0*.*\0");
		ofn.nFilterIndex		= 1;
		ofn.lpstrFileTitle		= NULL;
		ofn.nMaxFileTitle		= 0 ;
		TCHAR buffer[MAX_PATH];
		GetCurrentDirectory(MAX_PATH,buffer);
		//string dir_name = "data";
		ofn.lpstrInitialDir = buffer;
		//ofn.lpstrInitialDir		= NULL ;
		ofn.Flags				= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		GetOpenFileName(&ofn);    
		LPWSTR local_filename = ofn.lpstrFile;   

		if (local_filename && local_filename[0] != '\0')
		{
			ElapasedTime timer;
			timer.start();
			string obj_file_name = ws2s(local_filename);
			string texture_distance_file_name = ExePath() + string("\\..\\data\\pic\\texture_distance_map.bmp");
			wxLogMessage(ExePath().c_str());
			wxLogMessage((obj_file_name.c_str()));
			wxLogMessage((texture_distance_file_name.c_str()));
			if (g_model_ptr != NULL) {
				delete g_model_ptr;
			}
			g_model_ptr = new CRichModel(obj_file_name);
			g_model_ptr->Preprocess();
			g_mesh.ReadCRichModel(*g_model_ptr);
			readTexture(texture_distance_file_name);
			string file_name = getFileNameFromPath(g_model_ptr->GetFullPathAndFileName());
			char buf[1024];
			sprintf(buf, ("#Vertices %d\n#Edges %d\n#Faces %d\n"), g_model_ptr->GetNumOfVerts(), g_model_ptr->GetNumOfEdges(),	g_model_ptr->GetNumOfFaces());
			//sprintf(buf, ("#Vertices %d\n#Faces %d\n"), g_model_ptr->GetNumOfVerts(), g_model_ptr->GetNumOfFaces());
			message = buf;
		}
	}

	void toViewingModel()
	{
		mouse_click_mode = NONE;
	}

	void toSingleSource()
	{
		mouse_click_mode = SELECT_POINT;
	}
	void toMultipleSources()
	{
		mouse_click_mode = ADD_POINT;
	}

	bool find_hit(float x , float y , rms::Frame3f& v_hit_frame, int& hit_tri_id)
	{
		// find hit point
		const unsigned int * pViewport = g_pCamera->GetViewport();
		Wml::Ray3f ray;
		g_pCamera->GetPickRay( x, y, 
			pViewport[2], pViewport[3], ray.Origin, ray.Direction );
		return g_mesh.FindHitFrame(ray , v_hit_frame, hit_tri_id);
	}


	void mouseLeftDown(int x, int y,string& message)
	{
		if (g_model_ptr == NULL) return;
		rms::Frame3f v_hit_frame;
		int hit_tri_id = -1;
		bool bHit = find_hit(x, y, v_hit_frame, hit_tri_id);

		hit_point = CPoint3D(v_hit_frame.Origin().X(),
			v_hit_frame.Origin().Y(),
			v_hit_frame.Origin().Z());

		g_vLastMousePos = Wml::Vector2f(x,y);

		if( mouse_click_mode == SELECT_POINT) {
			//printf("select\n");
			wxLogMessage("select");
			char buf[1024];
			sprintf(buf,"tri id %d\n" , hit_tri_id);
			wxLogMessage(buf);
			printf("tri id %d\n", hit_tri_id);
			if (hit_tri_id >= 0) {
				//printf("hit_tri_id %d\n" , hit_tri_id);
				int pos = 0;
				double min_dis = 1e10;
				for (int j = 0; j < 3; ++j) {
					double dis = (g_model_ptr->Vert(g_model_ptr->Face(hit_tri_id)[j]) - hit_point).Len();
					if (dis <min_dis){
						min_dis = dis;
						pos = j;
					}
				}
				g_hit_vertex_id = g_model_ptr->Face(hit_tri_id)[pos];
				g_hit_vertices[0] = g_hit_vertex_id;
				char buf[1024];
				sprintf(buf,"Selected Vertex %d\n" , g_hit_vertex_id);
				message = buf;
				printf("Selected Vertex %d\n", g_hit_vertex_id);
				if (g_method_mode == METHOD_DISTANCE_FIELD) {
					g_distance_field_status = DISTANCE_FIELD_CHANGED;
				}
			}
		} else if(mouse_click_mode == ADD_POINT) {
			if (hit_tri_id >= 0) {
				int pos = 0;
				double min_dis = 1e10;
				for (int j = 0; j < 3; ++j) {
					double dis = (g_model_ptr->Vert(g_model_ptr->Face(hit_tri_id)[j]) - hit_point).Len();
					if (dis <min_dis){
						min_dis = dis;
						pos = j;
					}
				}
				int vert_id = g_model_ptr->Face(hit_tri_id)[pos];
				g_hit_vertices.push_back(vert_id);
				char buf[1024];
				sprintf(buf,"Selected Vertices:\n");
				message = buf;
				for (auto v:g_hit_vertices) {
					sprintf(buf,"%d\n" , v);
					message += buf;
				}
				if (g_method_mode == METHOD_DISTANCE_FIELD) {
					g_distance_field_status = DISTANCE_FIELD_CHANGED;
				}
			}
		}


	}

	void DGG_initial(const string& dgg_file_name)
	{
		if (g_svg_graph != NULL) {
			delete g_svg_graph;
			g_svg_graph = NULL;
		}
		g_distance_field_status = DISTANCE_FIELD_UNINITIAL;
		g_graph_type = GRAPH_DGG;
#if 0
		g_svg_graph = new LC_HY<float>();
		g_svg_graph->read_DGG_file_with_angle(dgg_file_name);
#else 
		g_svg_graph = new LC_LLL<float>();
		g_svg_graph->readDGGFileNotLoadAngle(dgg_file_name);
#endif
	}

	void continuousPickSingleSource(int x, int y)
	{
		if( mouse_click_mode == SELECT_POINT) {
			rms::Frame3f v_hit_frame;
			int hit_tri_id = -1;
			bool bHit = find_hit(x, y, v_hit_frame, hit_tri_id);

			hit_point = CPoint3D(v_hit_frame.Origin().X(),
				v_hit_frame.Origin().Y(),
				v_hit_frame.Origin().Z());

			g_vLastMousePos = Wml::Vector2f(x,y);

			//printf("select\n");
			wxLogMessage("select");
			char buf[1024];
			sprintf(buf,"tri id %d\n" , hit_tri_id);
			wxLogMessage(buf);
			if (hit_tri_id >= 0) {
				//printf("hit_tri_id %d\n" , hit_tri_id);
				int pos = 0;
				double min_dis = 1e10;
				for (int j = 0; j < 3; ++j) {
					double dis = (g_model_ptr->Vert(g_model_ptr->Face(hit_tri_id)[j]) - hit_point).Len();
					if (dis <min_dis){
						min_dis = dis;
						pos = j;
					}
				}
				g_hit_vertex_id = g_model_ptr->Face(hit_tri_id)[pos];
				g_hit_vertices[0] = g_hit_vertex_id;

				if (g_method_mode == METHOD_DISTANCE_FIELD) {
					g_distance_field_status = DISTANCE_FIELD_CHANGED;
				}
			}
		}
	}

	void LoadSVG(int& k,string& message);
	void LoadDGG(double& eps, string& message);
	void ComputeFD(double& eps, int& thread_num, string& message);
	void ComputeDGG(double& eps, int& thread_num, string& message);
	void ComputeSVG(int k, int& thread_num, string& message);
};


