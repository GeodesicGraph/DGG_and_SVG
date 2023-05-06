#ifndef _DGG_PRECOMPUTE_H_
#define _DGG_PRECOMPUTE_H_
#include "stdafx.h"

void DGG_precompute_ich_multithread_before_pruning(const string& input_obj_name, double eps_vg, string& DGG_file_name, double const_for_theta,
	int thread_num, double& ich_multi_time, int method, int fixed_K);

void DGG_precompute_ich_multithread(const string& input_obj_name, double eps_vg, string& DGG_file_name, double const_for_theta, int thread_num, int method, int fixed_K);

#endif
