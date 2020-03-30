#include <string>
#include <ctime>
#include <windows.h>
#include <cstdio>   

#include "dgg_time.h"
#include "dgg_dijkstra.h"
#include "ICH/RichModel.h"

using namespace std;

void readSources(const string& file_name, vector<int>& vector_file)
{
	int n;
	FILE* file = fopen(file_name.c_str(), "r");
	fscanf(file, "%d", &n);
	vector_file.clear();
	vector_file.resize(n);
	int d;
	for (int i = 0; i < n; ++i) {
		fscanf(file, "%d", &d);
		vector_file[i] = d;
	}
	fclose(file);
}

void writeVectorToFile(const string& file_name, const vector<double>& vec)
{
	FILE* file = fopen(file_name.c_str(), "w");
	fprintf(file, "%d\n", vec.size());
	for (int i = 0; i < vec.size(); ++i) {
		fprintf(file, "%.20lf\n", vec[i]);
	}
	fclose(file);
}

template <class T>
void get_distance(const string& obj_file_name, const string& DGG_file_name, const string& source_filename, const string& out_filename)
{
	const string method = "dij";
	CRichModel rich_model(obj_file_name);
	rich_model.Preprocess();
	SparseGraph<T>* s_graph = NULL;
	if (method == "dij") {
		s_graph = new Dijkstra<T>();
		s_graph->readDGGFile((string)DGG_file_name);
	}
	else {
		fprintf(stdout, "!!! Invalid choice !!!\n");
		exit(1);
	}
	double max_dis = 0;

	vector<int> sources;
	readSources(source_filename, sources);
	double average_time = 0;
	ElapasedTime t;
	s_graph->findShortestDistance(sources);	//find the geodesic distance from a single source to all vertex of a mesh
	average_time += t.getTime();
	vector<double> current_dis(rich_model.GetNumOfVerts());
	for (int i = 0; i < rich_model.GetNumOfVerts(); ++i) {
		current_dis[i] = s_graph->distanceToSource(i);
	}
	//STDERR Solving time
	fprintf(stderr, "%lf\n", average_time);
	writeVectorToFile(out_filename, current_dis);
}

template <class T>
void get_distance(const string& obj_file_name, const string& DGG_file_name, int source_vert, const string& out_filename)
{
	const string method = "dij";
	CRichModel rich_model(obj_file_name);
	rich_model.Preprocess();
	T al = 1.0;
	SparseGraph<T>* s_graph = NULL;
	if (method == "dij") {
		s_graph = new Dijkstra<T>();

		s_graph->readDGGFile((string)DGG_file_name);
	}
	else {
		fprintf(stdout, "!!! Invalid choice !!!\n");
		exit(1);
	}
	double max_dis = 0;

	double average_time = 0;
	ElapasedTime t;
	s_graph->findShortestDistance(source_vert);	//find the geodesic distance from a single source to all vertex of a mesh
	average_time += t.getTime();
	vector<double> current_dis(rich_model.GetNumOfVerts());
	for (int i = 0; i < rich_model.GetNumOfVerts(); ++i) {
		current_dis[i] = s_graph->distanceToSource(i);
	}
	//STDERR Solving time
	fprintf(stderr, "%.10lf\t", average_time);
	writeVectorToFile(out_filename, current_dis);
}

int main(int argc, char** argv)
{
	string obj_file_name;
	string dgg_file_name;
	if (string(argv[1]) == "SSAD") {
		obj_file_name = argv[2];
		dgg_file_name = argv[3];
		int source_vert = atoi(argv[4]);
		string out_file_name = argv[5];
		get_distance<float>(obj_file_name, dgg_file_name, source_vert, out_file_name);
	}
	else if (string(argv[1]) == "MSAD") {
		obj_file_name = argv[2];
		dgg_file_name = argv[3];
		string source_file = argv[4];
		string out_file_name = argv[5];
		get_distance<float>(obj_file_name, dgg_file_name, source_file, out_file_name);
	}
	return 0;
}