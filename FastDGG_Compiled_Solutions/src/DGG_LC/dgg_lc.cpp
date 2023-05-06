#include <string>
#include <ctime>
#include <random>
#include <windows.h>
#include <psapi.h>
#include <stdio.h>   
#include <tchar.h>
#include <cmath>
#include <cfloat>
#include "DGG_time.h"
#include "DGG_dijkstra.h"
#include "DGG_definition.h"

using namespace std;

enum FloatType
{
	DGG_FLOAT, DGG_DOUBLE
};

void output_error_distribute(double max_dis, const vector<pair<double, double>>& errors_list, double& all_average_error)
{
	all_average_error = 0;
	double longest_dis = max_dis;
	int interval_num = 100;
	vector<double> average_error_sep(interval_num + 1, 0.0);
	vector<double> average_error_cnt(interval_num + 1, 0);

	for (int i = 0; i < errors_list.size(); ++i) {
		double dis = errors_list[i].first;
		double error = errors_list[i].second;
		double percent = dis / longest_dis;
		int pos = percent * interval_num;
		average_error_sep[pos] += error;
		average_error_cnt[pos]++;
		all_average_error += errors_list[i].second;
	}

}

void readSources(const string& file_name, vector<int>& vector_file)
{
	int n;
	FILE* file = fopen(file_name.c_str(), "r");
	fscanf(file, "%d", &n);
	vector_file.clear();
	vector_file.resize(n);
	for (int i = 0; i < n; ++i) {
		int d;
		//fscanf(file, "%*d%d", &d);
		fscanf(file, "%d", &d);
		//d -= 1;
		vector_file[i] = d;
	}
	fclose(file);
}

void readVectorFromFile(const string& file_name, vector<double>& vector_file)
{
	int n;
	FILE* file = fopen(file_name.c_str(), "r");
	fscanf(file, "%d", &n);
	vector_file.clear();
	vector_file.resize(n);
	for (int i = 0; i < n; ++i) {
		double d;
		fscanf(file, "%lf", &d);
		//fprintf(stderr, "%lf\n", d);
		vector_file[i] = d;
	}
	fclose(file);
}

void writeVectorToFile(const string& file_name, const vector<double>& vec)
{
	FILE* file = fopen(file_name.c_str(), "w");
	fprintf(file, "%d\n", vec.size());
	for (int i = 0; i < vec.size(); ++i) {
		fprintf(file, "%lf\n", vec[i]);
	}
	fclose(file);
}

void writeMaxToFile(CRichModel& model, vector<double>& max_heat_dis, string filename)
{
	vector<pair<double, double>> texture;
	for (double d : max_heat_dis) {
		texture.push_back(make_pair(d, d));
	}
	model.FastSaveObjFile(filename, texture);
}

template<class T>
void check_errors(const string& obj_file_name, const string& DGG_file_name, const string& method, const string& source_filename, const string& exact_dis_file)
{
	CRichModel rich_model(obj_file_name);
	rich_model.Preprocess();

	SparseGraph<T>* s_graph = NULL;
	if (method == "dij") {
		//s_graph = new Dijstra_vector<T>();
		s_graph = new LC_LLL<T>();
		s_graph->readDGGFileNotLoadAngle((string)DGG_file_name);
	}
	else{
		fprintf(stderr, "invalid choice\n");
		exit(1);
	}

	vector<pair<double, double>> errors_list;	//<dis,error>
	//std::mt19937 rng;
	//std::uniform_int_distribution<int> uint_dist(0, rich_model.GetNumOfVerts() - 1);
	double average_time = 0;
	
	double max_dis = 0;

	//vector<double> correct_dis;
	//readVectorFromFile(exact_dis_file, correct_dis);

	vector<int> sources;
	//std::cout << "read source\n" ;
	//system("pause");
	readSources(source_filename, sources);
	int iteration_times = sources.size();
	//std::cout << "sources query";
	for(int i =0; i<sources.size();i++ )
	{
		int source_vert = sources[i];
		
		//std::cout << source_vert << std::endl;
		//std::cin >> source_vert;
		//system("pause");
		ElapasedTime t;
		s_graph->findShortestDistance(source_vert);	//find the geodesic distance from a single source to all vertex of a mesh
		average_time += t.getTime();
		//int inp;
		//std::cin >> inp;
		t.start();

		/*vector<double> current_dis(rich_model.GetNumOfVerts());
		for (int i = 0; i < rich_model.GetNumOfVerts(); ++i) {
			current_dis[i] = s_graph->distanceToSource(i);
		}*/
		//writeMaxToFile(rich_model, current_dis, obj_file_name.substr(0, obj_file_name.length() - 4) + "_dis.obj");
		// Output Geodesic Distances
		obj_file_name.length();
		string name = obj_file_name.substr(0, obj_file_name.length() - 4);
		string source = to_string(source_vert);
		string outputname = name + "_" + source + ".txt";
		const char* outputMesh = outputname.c_str();
		/*for (int j = 0; j < obj_file_name.length()-4; j++) {
			outputMesh[j] = obj_file_name[j];
		}
		outputMesh[obj_file_name.length() - 4] = '_';
		string src = to_string(source_vert);
		for (int j = obj_file_name.length() - 3; j < src.length() + obj_file_name.length() - 3; j++) {
			outputMesh[j] = src[j];
		}
		outputMesh[src.length() + obj_file_name.length() - 3] = '\0' ;*/
		if (outputMesh[0] != '\0')
		{
			ofstream output(outputMesh);
			//output << mesh.vertices().size() << endl;
			for (unsigned i = 0; i<rich_model.GetNumOfVerts(); ++i)
			{
				double distance = s_graph->distanceToSource(i);

				output << setprecision(20) << distance << endl;		//print geodesic distance for every vertex
			}

			output << endl;

			output.close();
		}
		//printf("write max\n");
		//fprintf(stderr,"correct sz %d vert %d\n", correct_dis.size(), rich_model.GetNumOfVerts());
//		assert(correct_dis.size() == rich_model.GetNumOfVerts());
		//max_dis = std::max(max_dis, *std::max_element(correct_dis.begin(), correct_dis.end()));

		/*int cnt_error_dis = 0;
		double average_error = 0;
		int cnt_finite = 0;
		for (int i = 0; i < correct_dis.size(); ++i) {
			double dis = current_dis[i];
			if (fabs(correct_dis[i]) < 1e-12) continue;
			if (_isnan(correct_dis[i]) || _isnan(dis)) continue;
			if (!_finite(correct_dis[i]) || !_finite(dis)) continue;
			double error = fabs(dis - correct_dis[i]) / correct_dis[i];
			if (error > 1) {
				cnt_error_dis++;
				continue;
			}
			cnt_finite++;
			average_error += error;
			errors_list.push_back(make_pair(correct_dis[i], error));
		}*/
	}
	//double max_error = FLT_MIN;
	//for (auto& e : errors_list) max_error = std::max(max_error, e.second);
	//double all_average_error;
	//output_error_distribute(max_dis, errors_list, all_average_error);

	fprintf(stderr, "ssad time %lf\n", average_time / iteration_times);
	system("pause");
	//fprintf(stderr, "average error %.20lf\n", all_average_error / errors_list.size());
	//fprintf(stderr, "max error %.20lf\n", max_error);

	HANDLE current_process = GetCurrentProcess();
	PROCESS_MEMORY_COUNTERS pmc;

	if (GetProcessMemoryInfo(current_process, &pmc, sizeof(pmc)))
	{
		fprintf(stderr, "PeakWorkingSetSize: %d MB\n", pmc.PeakWorkingSetSize / 1024 / 1024);
	}

	delete s_graph;
}

int main(int argc, char** argv)
{
	string obj_file_name;
	string DGG_file_name;
	FloatType float_type{ DGG_DOUBLE };
	if (argc < 6) {
		fprintf(stderr, "parameter insufficient !\n usage DGG_LC.exe [obj_file] [DGG_file] [source_file] [exact_dis_file] [dij or lc] [flt or dbl]\n");
		return 1;
	}
	else if (argc == 6) {
		if (string(argv[5]) == "flt") {
			float_type = DGG_FLOAT;
		}
		else if (string(argv[5]) == "dbl") {
			float_type = DGG_DOUBLE;
		}
		else {
			printf("type error!\n");
			exit(1);
		}
	}

	obj_file_name = argv[1];
	DGG_file_name = argv[2];
	string source_file = argv[3];
	string exact_dis_file;// = argv[4];
	string method = argv[4];
	if (float_type == DGG_FLOAT) {
		check_errors<float>(obj_file_name, DGG_file_name, method, source_file, exact_dis_file);
	}
	else if (float_type == DGG_DOUBLE) {
		check_errors<double>(obj_file_name, DGG_file_name, method, source_file, exact_dis_file);
	}

	return 0;
}