#include "ConfigLib/configfile.h"
#include "ConfigLib/configitem.h"
#include "Viewer.h"
using namespace configlib;

void Viewer::LoadDGG(double& eps, string& message)
{
	OPENFILENAME ofn;
	TCHAR szFile[MAX_PATH]  = _T("\0");
	ZeroMemory(&ofn , sizeof(ofn));
	ofn.lStructSize			= sizeof(ofn);
	//ofn.hwndOwner			=  FindWindow(NULL, "Mesh-Viewer");	//find main window to freeze it
	ofn.lpstrFile			= szFile ;
	//ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile			= sizeof(szFile);
	ofn.lpstrFilter			= _T("Model file (.config)\0*.config\0All\0*.*\0");
	ofn.nFilterIndex		= 1;
	ofn.lpstrFileTitle		= NULL ;
	ofn.nMaxFileTitle		= 0 ;
	string dir_name = ExePath();
	ofn.lpstrInitialDir = s2ws(dir_name).c_str();
	ofn.Flags				= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	GetOpenFileName(&ofn);    
	LPWSTR config_filename = ofn.lpstrFile;   

	if (config_filename && config_filename[0] != '\0')
	{
		ElapasedTime timer;
		timer.start();

		configfile config_file(ws2s(config_filename));

		configitem<std::string> conf_obj_file_name    (config_file , "main" , "obj_file_name"     , "o=","");
		configitem<std::string> conf_dgg_file_name    (config_file , "main" , "dgg_file_name"     , "s=","");
		configitem<double> conf_eps_vg (config_file, "main", "eps_vg","s=",0.01);
		configitem<std::string> conf_texture_filename    (config_file , "main" , "texture_distance_file_name"     , "t=","");

		config_file.read();
		//cout << string(conf_obj_file_name) <<" __\n";
		string obj_filename = toFullPath(conf_obj_file_name);
		//printf("obj_filename %s\n" , obj_filename.c_str());
		g_eps_vg = conf_eps_vg;
		eps = conf_eps_vg;
		string dgg_filename = toFullPath(conf_dgg_file_name);
		string texture_filename = toFullPath(conf_texture_filename);
		readTexture(texture_filename);

		if (g_model_ptr != NULL) {
			delete g_model_ptr;
		}
		g_model_ptr = new CRichModel(obj_filename);
		g_model_ptr->Preprocess();
		g_mesh.ReadCRichModel(*g_model_ptr);

		DGG_initial(dgg_filename);
		char buf[1024];
		string model_name = getFileNameFromPath(g_model_ptr->GetFullPathAndFileName());
		string svg_filename_without_path = getFileNameFromPath(conf_dgg_file_name);

		sprintf(buf, ("#Vertices %d\n#Edges %d\nAvg. Degree = %lf\n¦Å = %lf\n"),
			g_svg_graph->NodeNum(),
			g_svg_graph->num_edges_,
			g_svg_graph->average_degree_,
			g_eps_vg
			);
		message = buf;


	}

}

void Viewer::ComputeDGG(double& eps, int& thread_num, string& message)
{
	if (g_model_ptr != NULL) {
		char buf[1024];
		g_eps_vg = eps;
		double eps_vg = g_eps_vg;
		double theta = asin(sqrt(eps_vg)) / M_PI * 180;
		//double const_for_theta = 30.0 / theta;
		double const_for_theta = 5.0;
		string input_obj_name = g_model_ptr->GetFullPathAndFileName();
		//int thead_num = 4;
		sprintf(buf,"%s\\GeodesicGraph.exe %s %lf d %.0lf %d 2>> inf.txt", g_exe_dir.c_str(),  input_obj_name.c_str(), eps_vg, const_for_theta, thread_num);
		printf("%s\n",buf);
		system(buf);
		char buf_new[1024];
		sprintf(buf_new, "%s_DGG%.10lf_c%.0lf_pruning.binary", input_obj_name.substr(0, input_obj_name.length() - 4).c_str(), eps_vg, const_for_theta);
		//wxLogMessage("line 95");
		string dgg_file_name = string(buf_new);
		//wxLogMessage(dgg_file_name.c_str());

		double propagation_time=0;
		double pruning_time=0;

		char buf_xx[1024];
		sprintf(buf_xx,"%s_DGG%.10lf_c%.0lf.time", input_obj_name.substr(0,input_obj_name.length() - 4 ).c_str(), eps_vg, const_for_theta);
		string svg_time_filename = buf_xx;
		FILE* svg_time_file = fopen(svg_time_filename.c_str(), "r");
		assert(svg_time_file != NULL);
		fscanf(svg_time_file, "%lf" , &propagation_time);
		fscanf(svg_time_file, "%lf" , &pruning_time);

		string prefix = input_obj_name.substr(0, input_obj_name.length() - 4);
		string config_filename = prefix + "_DGG_" + to_string(eps_vg) + ".config";
		configfile config_file(config_filename.c_str());
		configitem<std::string> conf_obj_file_name    (config_file , "main" , "obj_file_name"     , "o=","");
		conf_obj_file_name = toRelativePath(input_obj_name);
		configitem<double> conf_eps_vg (config_file, "main", "eps_vg","s=",g_eps_vg);
		conf_eps_vg = g_eps_vg;
		configitem<std::string> conf_dgg_file_name    (config_file , "main" , "dgg_file_name"     , "s=","");
		conf_dgg_file_name = toRelativePath(dgg_file_name);
		configitem<std::string> conf_texture_distance_file_name    (config_file , "main" , "texture_distance_file_name"     , "t=","");

		conf_texture_distance_file_name = "..\\data\\pic\\texture_distance_map.bmp";
		config_file.write();

		DGG_initial(dgg_file_name);

		sprintf(buf, ("#Vertices %d\n#Edges %d\nAvg. Degree = %lf\n¦Å = %lf\nTime for computing candidate edges\n   %.2lf seconds\nTime for edge pruning\n   %.2lf seconds\n"),
			g_svg_graph->NodeNum(),
			g_svg_graph->num_edges_,
			g_svg_graph->average_degree_,
			g_eps_vg,
			propagation_time,
			pruning_time
			);

		message = buf;

	}

}

void Viewer::ComputeFD(double& eps, int& thread_num, string& message)
{
	if (g_model_ptr != NULL) {
		char buf[1024];
		g_eps_vg = eps;
		double eps_vg = g_eps_vg;
		double theta = asin(sqrt(eps_vg)) / M_PI * 180;
		//double const_for_theta = 30.0 / theta;
		double const_for_theta = 5.0;
		string input_obj_name = g_model_ptr->GetFullPathAndFileName();
		//int thead_num = 4;
		sprintf(buf, "%s\\GeodesicGraph.exe %s %lf f %.0lf %d 2>> inf.txt", g_exe_dir.c_str(), input_obj_name.c_str(), eps_vg, const_for_theta, thread_num);
		printf("%s\n", buf);
		system(buf);
		char buf_new[1024];
		sprintf(buf_new, "%s_FD%.10lf_c%.0lf.binary", input_obj_name.substr(0, input_obj_name.length() - 4).c_str(), eps_vg, const_for_theta);
		//wxLogMessage("line 95");
		string dgg_file_name = string(buf_new);
		//wxLogMessage(dgg_file_name.c_str());

		double propagation_time = 0;

		char buf_xx[1024];
		sprintf(buf_xx, "%s_FD%.10lf_c%.0lf.time", input_obj_name.substr(0, input_obj_name.length() - 4).c_str(), eps_vg, const_for_theta);
		string svg_time_filename = buf_xx;
		FILE* svg_time_file = fopen(svg_time_filename.c_str(), "r");
		assert(svg_time_file != NULL);
		fscanf(svg_time_file, "%lf", &propagation_time);

		string prefix = input_obj_name.substr(0, input_obj_name.length() - 4);
		string config_filename = prefix + "_FD_" + to_string(eps_vg) + ".config";
		configfile config_file(config_filename.c_str());
		configitem<std::string> conf_obj_file_name(config_file, "main", "obj_file_name", "o=", "");
		conf_obj_file_name = toRelativePath(input_obj_name);
		configitem<double> conf_eps_vg(config_file, "main", "eps_vg", "s=", g_eps_vg);
		conf_eps_vg = g_eps_vg;
		configitem<std::string> conf_dgg_file_name(config_file, "main", "dgg_file_name", "s=", "");
		conf_dgg_file_name = toRelativePath(dgg_file_name);
		configitem<std::string> conf_texture_distance_file_name(config_file, "main", "texture_distance_file_name", "t=", "");

		conf_texture_distance_file_name = "..\\data\\pic\\texture_distance_map.bmp";
		config_file.write();

		DGG_initial(dgg_file_name);

		sprintf(buf, ("#Vertices %d\n#Edges %d\nAvg. Degree = %lf\n\u03B5 = %lf\nTime for computing candidate edges\n   %.2lf seconds\n"),
			g_svg_graph->NodeNum(),
			g_svg_graph->num_edges_,
			g_svg_graph->average_degree_,
			g_eps_vg,
			propagation_time
			);

		message = buf;

	}

}


void Viewer::LoadSVG(int& k ,string& message)
{

	OPENFILENAME ofn ;
	TCHAR szFile[MAX_PATH]  = _T("\0");
	ZeroMemory(&ofn , sizeof(ofn));
	ofn.lStructSize			= sizeof(ofn);
	//ofn.hwndOwner			=  FindWindow(NULL, "Mesh-Viewer");	//find main window to freeze it
	ofn.lpstrFile			= szFile ;
	//ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile			= sizeof(szFile);
	ofn.lpstrFilter			= _T("Model file (.config)\0*.config\0All\0*.*\0");
	ofn.nFilterIndex		= 1;
	ofn.lpstrFileTitle		= NULL ;
	ofn.nMaxFileTitle		= 0 ;
	//string dir_name = ExePath();
	//TCHAR buffer[MAX_PATH];
	//GetCurrentDirectory(MAX_PATH,buffer);
	//string dir_name = "data";
	ofn.lpstrInitialDir = s2ws(ExePath()).c_str();
	ofn.Flags				= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	GetOpenFileName(&ofn);    
	LPWSTR config_filename = ofn.lpstrFile;   

	if (config_filename && config_filename[0] != '\0')
	{
		ElapasedTime timer;
		timer.start();
		configfile config_file(ws2s(config_filename));
		configitem<std::string> conf_obj_file_name    (config_file , "main" , "obj_file_name"     , "o=","");
		configitem<std::string> conf_svg_file_name    (config_file , "main" , "svg_file_name"     , "s=","");
		configitem<int> conf_svg_k (config_file, "main", "svg_k","s=",50);

		configitem<std::string> conf_texture_filename    (config_file , "main" , "texture_distance_file_name"     , "t=","");

		config_file.read();

		string obj_filename = toFullPath(conf_obj_file_name);
		wxLogMessage(((string)conf_obj_file_name).c_str());
		wxLogMessage((obj_filename.c_str()));
		printf("obj_filename %s\n" , obj_filename.c_str());
		g_svg_k = conf_svg_k;
		k = g_svg_k;
		string svg_filename = toFullPath(conf_svg_file_name);
		string texture_filename = toFullPath(conf_texture_filename);
		CRichModel model(obj_filename);
		if (g_model_ptr != NULL) {
			delete g_model_ptr;
		}
		g_model_ptr = new CRichModel(obj_filename);
		g_model_ptr->Preprocess();
		g_mesh.ReadCRichModel(*g_model_ptr);
		wxLogMessage((obj_filename.c_str()));

		readTexture(texture_filename);
		if (g_svg_graph != NULL) {
			delete g_svg_graph;
			g_svg_graph = NULL;
		}
		g_graph_type = GRAPH_SVG;
		g_svg_graph = new LC_LLL<float>(); 
		g_svg_graph->readDGGFileNotLoadAngle(svg_filename);
		wxLogMessage((svg_filename.c_str()));
		g_distance_field_status = DISTANCE_FIELD_UNINITIAL;
		char buf[1024];
		string model_name = getFileNameFromPath(g_model_ptr->GetFullPathAndFileName());
		string svg_filename_without_path = getFileNameFromPath(conf_svg_file_name);
		//sprintf(buf, ("Loaded %s\n#vertices %d\n#faces%d\n#mesh edges%d\n\nLoaded SVG file %s\n#SVG edges %d\n#SVG Vertices %d\nK = %d\nDegree = %lf\n"),
		//	(model_name).c_str(),	
		//	g_model_ptr->GetNumOfVerts(),	
		//	g_model_ptr->GetNumOfFaces(),
		//	g_model_ptr->GetNumOfEdges(),
		//	((string)svg_filename_without_path).c_str(),
		//	g_svg_graph->num_edges_,
		//	g_svg_graph->NodeNum(),
		//	g_svg_k,
		//	g_svg_graph->average_degree_);
		sprintf(buf, ("#Vertices %d\n#Edges %d\nAvg. Degree = %lf\nK = %d\n"),
			g_svg_graph->NodeNum(),
			g_svg_graph->num_edges_,
			g_svg_graph->average_degree_,
			g_svg_k
			);
		message = buf;
	}
}

void Viewer::ComputeSVG(int k, int& thread_num, string& message)
{
	if (g_model_ptr != NULL) {
		char buf[1024];
		string input_obj_name = g_model_ptr->GetFullPathAndFileName();
		string prefix = input_obj_name.substr(0,input_obj_name.length()-4);
		g_svg_k = k;
		sprintf(buf, "%s\\GeodesicGraph.exe %s %d s %d 2>> inf.txt", g_exe_dir.c_str(), input_obj_name.c_str(), k, thread_num);
		printf("%s\n",buf);
		system(buf);

		string svg_file_name = prefix + "_SVG_K" + to_string(g_svg_k) +  ".binary";
		string svg_time_filename = prefix + "_SVG_K" + to_string(g_svg_k) +  ".time";
		FILE* svg_time_file = fopen(svg_time_filename.c_str(), "r");
		double svg_time;
		fscanf(svg_time_file, "%lf" , &svg_time);

		fclose(svg_time_file);

		string config_filename = prefix + "_svg_k_" + to_string(g_svg_k) + ".config";

		configfile config_file(config_filename.c_str());
		configitem<std::string> conf_obj_file_name    (config_file , "main" , "obj_file_name"     , "o=","");
		conf_obj_file_name = toRelativePath((string)input_obj_name);
		configitem<int> conf_svg_k (config_file, "main", "svg_k","s=",50);
		conf_svg_k = g_svg_k;
		configitem<std::string> conf_svg_file_name    (config_file , "main" , "svg_file_name"     , "s=","");
		conf_svg_file_name = toRelativePath(svg_file_name);
		configitem<std::string> conf_texture_distance_file_name    (config_file , "main" , "texture_distance_file_name"     , "t=","");

		conf_texture_distance_file_name = "..\\data\\pic\\texture_distance_map.bmp";
		config_file.write();

		if (g_svg_graph != NULL) {
			delete g_svg_graph;
			g_svg_graph = NULL;
		}
		g_graph_type = GRAPH_SVG;
		g_svg_graph = new LC_LLL<float>(); 
		g_svg_graph->readDGGFileNotLoadAngle(svg_file_name);
		g_distance_field_status = DISTANCE_FIELD_UNINITIAL;
		string model_name = getFileNameFromPath(g_model_ptr->GetFullPathAndFileName());
		string svg_filename_without_path = getFileNameFromPath(conf_svg_file_name);
		//sprintf(buf, ("Loaded %s\n#vertices %d\n#faces%d\n#mesh edges%d\n\nComputed SVG file %s\ntime %.2lfs\n#SVG edges %d\n#SVG Vertices %d\nK = %d\nDegree = %lf\n"),
		//	(model_name).c_str(),	
		//	g_model_ptr->GetNumOfVerts(),	
		//	g_model_ptr->GetNumOfFaces(),
		//	g_model_ptr->GetNumOfEdges(),
		//	((string)svg_filename_without_path).c_str(),
		//	svg_time,
		//	g_svg_graph->num_edges_,
		//	g_svg_graph->NodeNum(),
		//	g_svg_k,
		//	g_svg_graph->average_degree_);
		sprintf(buf, ("#Vertices %d\n#Edges %d\nAvg. Degree = %lf\nK = %d\nTime for computing candidate edges\n   %.2lf seconds\n"),
			g_svg_graph->NodeNum(),
			g_svg_graph->num_edges_,
			g_svg_graph->average_degree_,
			g_svg_k,
			svg_time
			);
		message = (string)buf;
	}

}