
#include "stdafx.h"
#include "DGG_precompute.h"

#include <psapi.h>
#include <conio.h>

#pragma comment(lib, "psapi.lib") //remember added it

int main(int argc, char** argv)
{
	string input_file_name;
	string method;
	double eps_vg;
	double const_for_theta = 5;
	int thread_num = 8;
	int fixed_K;
	if (argc >= 4) {
		method = argv[3];
		if (method == "d" || method == "f") {
			input_file_name = argv[1];
			eps_vg = atof(argv[2]);
			if (argc >= 5) {
				const_for_theta = atoi(argv[4]);
				if (argc == 6) {
					thread_num = atoi(argv[5]);
				}
				else {
					printf("must specify thread num, for example, xx.exe yy.obj 0.01 fm 5(const of fan-shape Dijkstra) 8(number of threads)\n");
					exit(1);
				}
			}
		}
		else if (method == "s") {
			input_file_name = argv[1];
			fixed_K = atoi(argv[2]);
			thread_num = atoi(argv[4]);
		}
	}
	else {
		fprintf(stderr, "wrong argument, usage example 'FastDGG_precompute.exe bunny.obj 0.01 f 5 8'");
		exit(1);
	}

	string DGG_file_name;
	
	
	if (method == "f") DGG_precompute_ich_multithread(input_file_name, eps_vg, DGG_file_name, const_for_theta, thread_num, 0, 0);
	else if (method == "d") DGG_precompute_ich_multithread(input_file_name, eps_vg, DGG_file_name, const_for_theta, thread_num, 1, 0);
	else if (method == "s") DGG_precompute_ich_multithread(input_file_name, eps_vg, DGG_file_name, const_for_theta, thread_num, 2, fixed_K);
	
	return 0;
}

