#include "stdafx.h"
#include "DGG_precompute.h"

using namespace std;

int main(int argc, char** argv)
{
	string input_file_name;
	string method;
	double accuracy_control_parameter = 0.01;
	double const_for_theta = 5;
	int thread_num = 1;
	int fixed_K = 20;
	if (argc >= 3) {
		method = argv[1];
		if (method == "f") {
			input_file_name = argv[2];
			accuracy_control_parameter = atof(argv[3]);
		}
		else if (method == "s") {
			input_file_name = argv[2];
			fixed_K = atoi(argv[3]);
		}
		if (argc >= 4) {
			thread_num = atoi(argv[4]);
		}
		else {
			printf("If not specify number of threads, pre-computing in single thread.\n");
		}
	}
	else {
		fprintf(stderr, "Wrong argument, usage example 'GeodesicGraphPrecompute.exe f bunny.obj 0.01 8'");
		exit(1);
	}

	if (method == "f") DGG_ICH_Precompute_Multithread(input_file_name, accuracy_control_parameter, const_for_theta, thread_num);
	else if (method == "s") SVG_ICH_Precompute_Multithread(input_file_name, fixed_K, thread_num);

	return 0;
}

