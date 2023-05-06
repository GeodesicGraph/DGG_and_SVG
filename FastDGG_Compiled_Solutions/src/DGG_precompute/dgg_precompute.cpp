#include "stdafx.h"
#include <windows.h>
#include "Shlwapi.h"
#include "dgg_time.h"
#include "dgg_buffer.h"
#include "DGG_precompute.h"
#include "ICH\RichModel.h"
#include "ICH\ICHWithFurtherPriorityQueue.h"
#include "ICH\ICHWithFurtherPriorityQueueForFD.h"

#include "dgg_definition.h"

#include <thread>
#include <random>

#include <psapi.h>
#include <conio.h>

#pragma comment(lib, "psapi.lib") //remember added it

void DGG_precompute_fixed_Dist(const string& input_obj_name){
	CRichModel model(input_obj_name);
	int noFaces = model.GetNumOfFaces();
	double area = 0;
	for (int i = 0; i < noFaces; i++){
		double a = model.Edge(model.GetFirstEdgeIndex(i)).length;
		double b = model.Edge(model.GetSecondEdgeIndex(i)).length;
		double c = model.Edge(model.GetThirdEdgeIndex(i)).length;
		double s = (a + b + c) / 2.0;
		area += sqrt(s*(s-a)*(s-b)*(s-c));
	}
	area = area / noFaces;
	double length = sqrt(area);
	double noVertexMet = 0;
	for (int i = 0; i < model.GetNumOfVerts(); i++){
		CICHWithFurtherPriorityQueue alg(model, vector < int > {i});
		set<int> fixed_dests;
		double distmax = 0.01*length;
		alg.ExecuteLocally_SVG(distmax, fixed_dests, 100000000);
		noVertexMet += fixed_dests.size() - 1.0;
		fixed_dests.clear();
	}	
	printf("No vertex met on average : %lf", noVertexMet / model.GetNumOfVerts());
}

void getDisAndAngles(const CRichModel& model, int source, double eps_vg, vector<int>& dests, vector<double>& angles, vector<double>& dis, int method, int fixed_K)
{
	if (method == 0){
		CICHWithFurtherPriorityQueueForFD alg(model, vector < int > {source});
		set<int> fixed_dests;
		alg.ExecuteLocally_FastDGG(eps_vg, fixed_dests);
		dests.clear();
		dests.reserve(fixed_dests.size());
		dis.reserve(fixed_dests.size());
		for (auto d : fixed_dests) {
			map<int, CICHWithFurtherPriorityQueueForFD::InfoAtVertex>::const_iterator it = alg.m_InfoAtVertices.find(d);
			int indexOfParent = it->second.indexOfParent;
			int parent = 0;
			if (it->second.fParentIsPseudoSource) {
				parent = indexOfParent;
			}
			else {
				parent = it->second.indexOfRootVertOfParent;
			}
			if (parent == source) {
				dests.push_back(d);
				dis.push_back(it->second.disUptodate);
			}
		}
		angles.reserve(dests.size());
		for (auto d : dests) {
			bool isVert;
			int id;
			CPoint3D t = alg.BackTraceDirectionOnly(d, isVert, id);
			auto& neighs = model.Neigh(source);
			vector<double> sum_angle(neighs.size() + 1);
			sum_angle[0] = 0;
			for (int j = 1; j <= neighs.size(); ++j) {
				sum_angle[j] = sum_angle[j - 1] + neighs[j - 1].second;
			}
			double angle = 0;
			if (isVert) {
				bool flag_found = false;
				for (int j = 0; j < neighs.size(); ++j) {
					auto& neigh = neighs[j];
					if (id == model.Edge(neigh.first).indexOfRightVert) {
						flag_found = true;
						angle = sum_angle[j];
						break;
					}
				}
				if (!flag_found) {
					angle = -1;
				}
			}
			else { // is edge
				int v0 = model.Edge(id).indexOfLeftVert;
				int v1 = model.Edge(id).indexOfRightVert;
				bool flag = false;
				for (int j = 0; j < neighs.size(); ++j) {
					auto& neigh = neighs[j];
					if (v0 == model.Edge(neigh.first).indexOfRightVert) {
						int jminus1 = (j - 1 + neighs.size()) % neighs.size();
						int vjminus1 = model.Edge(neighs[jminus1].first).indexOfRightVert;
						int jplus1 = (j + 1) % neighs.size();
						int vjplus1 = model.Edge(neighs[jplus1].first).indexOfRightVert;

						if (v1 == vjminus1) {//v1 first
							double l = model.Edge(neighs[jminus1].first).length;
							double r = (model.Vert(source) - t).Len();
							double b = (model.Vert(vjminus1) - t).Len();
							angle = sum_angle[jminus1] + acos((l * l + r * r - b * b) / (2 * l * r));
						}
						else if (v1 == vjplus1) {//v0 first
							double l = model.Edge(neighs[j].first).length;
							double r = (model.Vert(source) - t).Len();
							double b = (model.Vert(v0) - t).Len();
							angle = sum_angle[j] + acos((l * l + r * r - b * b) / (2 * l * r));
						}
						else{}
						flag = true;
						break;
					}
				}
				if (!flag) {
					//fprintf(stderr, "flag %d\n", flag);
					//printf("source %d\n", source);
					//printf("v0 %d v1 %d\n", v0, v1);
					//printBallToObj(vector < CPoint3D > {model.Vert(source)},"bunny_nf10k_source.obj",0.01);
					//printBallToObj(vector < CPoint3D > {model.Vert(v0),model.Vert(v1)}, "bunny_nf10k_edge_point.obj", 0.01);
					////printf("")
					//	for (int j = 0; j < neighs.size(); ++j) {
					//		auto& neigh = neighs[j];
					//		printf("right %d\n", model.Edge(neigh.first).indexOfRightVert);
					//	}
					angle = -1;
				}
			}
			angles.push_back(angle);
		}
	}
	else if (method == 1 || method == 2) {
		CICHWithFurtherPriorityQueue alg(model, vector < int > {source});
		set<int> fixed_dests;
		double max_radius = 1e10;
		if (method == 1) alg.ExecuteLocally_DGG(eps_vg, fixed_dests);
		else if (method == 2) alg.ExecuteLocally_SVG(max_radius, fixed_dests, fixed_K);
		dests.clear();
		dests.reserve(fixed_dests.size());
		dis.reserve(fixed_dests.size());
		for (auto d : fixed_dests) {
			map<int, CICHWithFurtherPriorityQueue::InfoAtVertex>::const_iterator it = alg.m_InfoAtVertices.find(d);
			int indexOfParent = it->second.indexOfParent;
			int parent = 0;
			if (it->second.fParentIsPseudoSource) {
				parent = indexOfParent;
			}
			else {
				parent = it->second.indexOfRootVertOfParent;
			}
			if (parent == source) {
				dests.push_back(d);
				dis.push_back(it->second.disUptodate);
			}
		}
		angles.reserve(dests.size());
		for (auto d : dests) {
			bool isVert;
			int id;
			CPoint3D t = alg.BackTraceDirectionOnly(d, isVert, id);
			auto& neighs = model.Neigh(source);
			vector<double> sum_angle(neighs.size() + 1);
			sum_angle[0] = 0;
			for (int j = 1; j <= neighs.size(); ++j) {
				sum_angle[j] = sum_angle[j - 1] + neighs[j - 1].second;
			}
			double angle = 0;
			if (isVert) {
				bool flag_found = false;
				for (int j = 0; j < neighs.size(); ++j) {
					auto& neigh = neighs[j];
					if (id == model.Edge(neigh.first).indexOfRightVert) {
						flag_found = true;
						angle = sum_angle[j];
						break;
					}
				}
				if (!flag_found) {
					angle = -1;
				}
			}
			else { // is edge
				int v0 = model.Edge(id).indexOfLeftVert;
				int v1 = model.Edge(id).indexOfRightVert;
				bool flag = false;
				for (int j = 0; j < neighs.size(); ++j) {
					auto& neigh = neighs[j];
					if (v0 == model.Edge(neigh.first).indexOfRightVert) {
						int jminus1 = (j - 1 + neighs.size()) % neighs.size();
						int vjminus1 = model.Edge(neighs[jminus1].first).indexOfRightVert;
						int jplus1 = (j + 1) % neighs.size();
						int vjplus1 = model.Edge(neighs[jplus1].first).indexOfRightVert;
						if (v1 == vjminus1) {//v1 first
							double l = model.Edge(neighs[jminus1].first).length;
							double r = (model.Vert(source) - t).Len();
							double b = (model.Vert(vjminus1) - t).Len();
							angle = sum_angle[jminus1] + acos((l * l + r * r - b * b) / (2 * l * r));
						}
						else if (v1 == vjplus1) {//v0 first
							double l = model.Edge(neighs[j].first).length;
							double r = (model.Vert(source) - t).Len();
							double b = (model.Vert(v0) - t).Len();
							angle = sum_angle[j] + acos((l * l + r * r - b * b) / (2 * l * r));
						}
						else{}
						flag = true;
						break;
					}
				}
				if (!flag) {
					angle = -1;
				}
			}
			angles.push_back(angle);
		}
	}
}

void getFanOutput(const vector<int>& dests, const vector<double>& angles,
				  const vector<double>& dis, const CRichModel& model,  
				  double theta, int source ,
				  vector<BodyPartOfDGGWithAngle>& body_parts_with_angle)
{

	body_parts_with_angle.reserve(dests.size());
	for (int i = 0; i < dests.size(); ++i) {
		if (angles[i] >= 0) {
			BodyPartOfDGGWithAngle b_with_angle(dests[i], dis[i], angles[i], 0, 0);
			body_parts_with_angle.push_back(b_with_angle);
		}
	}
	sort(body_parts_with_angle.begin(), body_parts_with_angle.end());
	double angle_sum = model.AngleSum(source);
	vector<double> tmp_angles(body_parts_with_angle.size() * 2);
	for (int i = 0; i < body_parts_with_angle.size(); ++i) {
		tmp_angles[i] = body_parts_with_angle[i].angle;
	}
	for (int i = body_parts_with_angle.size(); i < tmp_angles.size(); ++i) {
		tmp_angles[i] = body_parts_with_angle[i - body_parts_with_angle.size()].angle + angle_sum;
	}
	for (int i = 0; i < body_parts_with_angle.size(); ++i) {//assume i is father
		double father_angle = body_parts_with_angle[i].angle;
		//based on father_angle as 0
		double start_angle = M_PI - theta + father_angle;
		double end_angle = angle_sum - (M_PI - theta) + father_angle;
		if (start_angle > end_angle) {
			body_parts_with_angle[i].begin_pos = -1;
			body_parts_with_angle[i].end_pos = -1;
			continue;
		}

		int start_pos = lower_bound(tmp_angles.begin(), tmp_angles.end(), start_angle) - tmp_angles.begin();
		if (start_pos > 0) start_pos--;
		int end_pos = lower_bound(tmp_angles.begin(), tmp_angles.end(), end_angle) - tmp_angles.begin();
		//printf("start_angle %lf start_pos_angle %lf end_angle  %lf end_pos_angle %lf\n" , start_angle, tmp_angles[start_pos], end_angle, tmp_angles[end_pos]);
		if (start_pos >= body_parts_with_angle.size()) start_pos -= body_parts_with_angle.size();
		if (end_pos >= body_parts_with_angle.size()) end_pos -= body_parts_with_angle.size();
		body_parts_with_angle[i].begin_pos = start_pos;
		body_parts_with_angle[i].end_pos = end_pos;
	}
}

void ichPropogateHead(const HeadOfDGG& head, const string& part_DGG_filename, double eps_vg, double theta, const CRichModel& model, int thread_id, int method, int fixed_K)
{
	DGGBuffer dgg_buffer;
	dgg_buffer.open(part_DGG_filename);
	dgg_buffer.addStruct(&head, sizeof(head));
	ElapasedTime time_once;

	double average_degree = 0;
	for (int source = head.begin_vertex_index; source <= head.end_vertex_index; ++source) {
		if (thread_id == 1) {
			time_once.printEstimateTime(.05, (double) (source - head.begin_vertex_index) / (head.end_vertex_index - head.begin_vertex_index));
		}
		vector<int> dests;
		vector<double> angles;
		vector<double> dis;
		getDisAndAngles(model, source, eps_vg, dests, angles, dis, method, fixed_K);

		vector<BodyPartOfDGGWithAngle> body_parts_with_angle;
		getFanOutput(dests, angles, dis, model, theta, source, body_parts_with_angle);

		BodyHeadOfDGG body_header(source, body_parts_with_angle.size());
		dgg_buffer.addStruct((void*)&body_header, sizeof(body_header));
		for (auto& b : body_parts_with_angle) {
			dgg_buffer.addStruct((void*)&b, sizeof(b));
		}
		average_degree += body_parts_with_angle.size();
	}
	dgg_buffer.close();

}

void combinePartPrecomputeFiles(const vector<string>& part_filenames, const string& DGG_filename, int num_of_vertex, int thread_num)
{
	HeadOfDGG head;
	head.begin_vertex_index = 0; head.end_vertex_index = num_of_vertex - 1;
	head.num_of_vertex = num_of_vertex;
	ofstream output_file(DGG_filename.c_str(), ios::out | ios::binary);
	output_file.write((char*)&head, sizeof(head));

	for (int thread_id = 0; thread_id < thread_num; ++thread_id) {
		const string& part_DGG_filename = part_filenames[thread_id];
		std::ifstream input_file(part_DGG_filename, std::ios::in | std::ios::binary);
		HeadOfDGG head_of_DGG;
		input_file.read((char*)&head_of_DGG, sizeof(head_of_DGG));

		for (int i = head_of_DGG.begin_vertex_index; i <= head_of_DGG.end_vertex_index; ++i) {
			BodyHeadOfDGG body_head;
			input_file.read((char*)&body_head, sizeof(body_head));
			vector<BodyPartOfDGGWithAngle> body_parts;
			body_parts.reserve(body_head.neighbor_num);
			for (int j = 0; j < body_head.neighbor_num; ++j) {
				BodyPartOfDGGWithAngle body_part;
				input_file.read((char*)&body_part, sizeof(body_part));
				body_parts.push_back(body_part);
			}
			output_file.write((char*)&body_head, sizeof(body_head));
			for (auto& b : body_parts) {
				output_file.write((char*)&b, sizeof(b));
			}
		}
		input_file.close();
	}
	output_file.close();

	for (auto& p : part_filenames) {
		DeleteFile(p.c_str());
	}
}

void DGG_precompute_ich_multithread_before_pruning(const string& input_obj_name, double eps_vg,
												   string& DGG_file_name, double const_for_theta, 
												   int thread_num, double& ich_multi_time, int method, int fixed_K)
{
	char buf[1024];
	if (method == 0) sprintf(buf, "%s_FD%.10lf_c%.0lf.binary", input_obj_name.substr(0, input_obj_name.length() - 4).c_str(), eps_vg, const_for_theta);
	else if (method == 1) sprintf(buf, "%s_DGG%.10lf_c%.0lf.binary", input_obj_name.substr(0, input_obj_name.length() - 4).c_str(), eps_vg, const_for_theta);
	else if (method == 2) sprintf(buf, "%s_SVG_K%d.binary", input_obj_name.substr(0, input_obj_name.length() - 4).c_str(), fixed_K);
	DGG_file_name = string(buf);
#if 0
	int DGG_file_exists = PathFileExists(DGG_file_name.c_str());
	if (DGG_file_exists == 1)
	{
		return;
	}
#endif

	ElapasedTime total_t;
	double theta = asin(sqrt(eps_vg));
	theta *= const_for_theta;
	if (method == 2) fprintf(stderr, "\n%s threads %d K %d\n", input_obj_name.substr(0, input_obj_name.length() - 4).c_str(), thread_num, fixed_K);
	else fprintf(stderr, "\n%s threads %d eps %lf const %lf theta %lf\n", input_obj_name.substr(0, input_obj_name.length() - 4).c_str(), thread_num, eps_vg, const_for_theta, theta / M_PI * 180.0);
	CRichModel model(input_obj_name);
	model.Preprocess();

	vector<HeadOfDGG> heads;
	vector<string> DGG_part_file_names;
	ElapasedTime time_multi;
	int part_size = model.GetNumOfVerts() / thread_num;
	for (int i = 0; i < thread_num; ++i) {
		HeadOfDGG head;
		head.num_of_vertex = model.GetNumOfVerts();
		head.begin_vertex_index = i * part_size;
		if (i != thread_num - 1) {
			head.end_vertex_index = (i + 1)*part_size - 1;
		} else {
			head.end_vertex_index = model.GetNumOfVerts() - 1;
		}
		heads.push_back(head);
		char buf[1024];
		sprintf(buf, "%s_DGGICH%.10lf_c%.0lf_part%d.binary", input_obj_name.substr(0, input_obj_name.length() - 4).c_str(), eps_vg, const_for_theta, i);
		DGG_part_file_names.push_back((string)buf);
	}
	vector<std::thread> tt(thread_num);
	for (int i = 0; i < thread_num; ++i) {
		tt[i] = std::thread(&ichPropogateHead, heads[i], DGG_part_file_names[i], eps_vg, theta, std::ref(model), thread_num-i, method, fixed_K);
	}
	for (int i = 0; i < thread_num; ++i) {
		tt[i].join();
	}
	ich_multi_time = time_multi.getTime();
	time_multi.printTime("dgg/svg ich");
#if 0
	PROCESS_MEMORY_COUNTERS pmc;
	if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
	{
		//cout << "MEM USAGE: " << endl;
		fprintf(stderr, "DGG before pruning mem usage\n");
		cerr << " PeakWSS: " << pmc.PeakWorkingSetSize / 1024 << " KB " << endl;
		cerr << " WSS: " << pmc.WorkingSetSize / 1024 << " KB " << endl;
		cerr << " PFU: " << pmc.PagefileUsage / 1024 << " KB " << endl;
		cerr << " PeakPFU: " << pmc.PeakPagefileUsage / 1024 << " KB " << std::endl;
	}
#endif
	ElapasedTime combine_time;
	combinePartPrecomputeFiles(DGG_part_file_names, DGG_file_name, model.GetNumOfVerts(), thread_num);
	combine_time.printTime("combine");

}

template<class T>
void readInputFile(const string& DGG_file_name,
	vector<vector<int>>& graph_neighbor,
	vector<vector<T>>& graph_neighbor_dis,
	vector<vector<bool>>& graph_neighbor_deleted,
	int& node_number)
{

	std::ifstream input_file(DGG_file_name, std::ios::in | std::ios::binary);
	HeadOfDGG head_of_DGG;
	input_file.read((char*)&head_of_DGG, sizeof(head_of_DGG));
	node_number = head_of_DGG.num_of_vertex;
	graph_neighbor.reserve(node_number);
	graph_neighbor.resize(node_number);
	graph_neighbor_dis.reserve(node_number);
	graph_neighbor_dis.resize(node_number);
	graph_neighbor_deleted.reserve(node_number);
	graph_neighbor_deleted.resize(node_number);
	for (int i = 0; i < head_of_DGG.num_of_vertex; ++i) {
		BodyHeadOfDGG body_head;
		input_file.read((char*)&body_head, sizeof(body_head));
		std::vector<BodyPartOfDGGWithAngle> body_parts;
		for (int j = 0; j < body_head.neighbor_num; ++j) {
			BodyPartOfDGGWithAngle body_part;
			input_file.read((char*)&body_part, sizeof(body_part));
			body_parts.push_back(body_part);
		}
		int u = body_head.source_index;
		int number_of_neighbor = body_parts.size();
		graph_neighbor[u].reserve(number_of_neighbor);
		graph_neighbor_dis[u].reserve(number_of_neighbor);
		graph_neighbor_deleted[u].reserve(number_of_neighbor);
		for (auto body : body_parts) {
			graph_neighbor[u].push_back(body.dest_index);
			graph_neighbor_dis[u].push_back(body.dest_dis);
			graph_neighbor_deleted[u].push_back(false);
		}

	}
	input_file.close();
}

template <class T>
void dijkstra_pruning_induced_graph(const vector<vector<int>>&  graph_neighbor,
	const vector<vector<T>>& graph_neighbor_dis,
	vector<bool>& current_graph_neighbor_deleted,
	int src, double eps_vg, vector<T>& dis, vector<bool>& mark)
{
	const double max_error = 1e-5;
	struct QueueNode{
		T dis;
		int node_index;
		QueueNode(){}
		QueueNode(int _node_index, double _dis) {
			dis = _dis;
			node_index = _node_index;
		}
		bool operator<(const QueueNode& other)const{
			return dis > other.dis;
		}
	};
	priority_queue<QueueNode> que;
	dis[src] = 0;
	mark[src] = true;
	map<int, int> node_map;
	for (int i = 0; i < graph_neighbor[src].size(); ++i) {
		int v = graph_neighbor[src][i];
		T d = graph_neighbor_dis[src][i];
		dis[v] = d;
		que.push(QueueNode(v, d));
		node_map[v] = i;
	}
	int cnt = 0;
	while (!que.empty()) {
		QueueNode u = que.top();
		cnt++;
		que.pop();
		if (mark[u.node_index]) continue;
		mark[u.node_index] = true;
		bool found_flag = false;
		for (int i = 0; i < graph_neighbor[u.node_index].size(); ++i) {
			int v = graph_neighbor[u.node_index][i];
			T d = graph_neighbor_dis[u.node_index][i];
			if (fabs(dis[v] - JiajunMaxDist) < max_error || u.node_index == v) {
				continue;
			}
			if (u.dis + d < dis[v] * (1 + eps_vg)) {
				QueueNode b;
				b.node_index = v;
				b.dis = min(u.dis + d, dis[v]);
				dis[v] = b.dis;
				current_graph_neighbor_deleted[node_map[v]] = true;
				que.push(b);
			}
		}
	}
	for (int v : graph_neighbor[src]) {
		dis[v] = JiajunMaxDist;
		mark[v] = false;
	}
	dis[src] = JiajunMaxDist;
	mark[src] = false;
}

template<class T>
void dijkstraPruningThread(int thread_id, int thread_num, int node_number,
	const vector<vector<int>>& graph_neighbor,
	const vector<vector<T>>& graph_neighbor_dis,
	vector<vector<bool>>& graph_neighbor_deleted, double eps_vg)
{
	vector<T> dis(node_number);
	vector<bool> mark(node_number);
	fill(dis.begin(), dis.end(), JiajunMaxDist);
	fill(mark.begin(), mark.end(), false);
	int part_size = node_number / thread_num;
	int begin = thread_id * part_size;
	int end;
	if (thread_id != thread_num - 1) {
		end = (thread_id + 1) * part_size - 1;
	}
	else {
		end = node_number - 1;
	}
	ElapasedTime time_once;
	double past_time = 0;
	for (int i = begin; i <= end; ++i) {
		if (time_once.getTime() - past_time > 5) {
			past_time = time_once.getTime();
			char buf[128];
			double percent = (double)(i - begin)  * 100. / double(end - begin + 1);
			double current_time = time_once.getTime();
			double remain_time = current_time / percent * (100 - percent);
			printf("Computed %.0lf percent, time %lf, estimate_remain_time %lf\n",
				percent, current_time, remain_time);
		}

		dijkstra_pruning_induced_graph<T>(graph_neighbor,
			graph_neighbor_dis, graph_neighbor_deleted[i], i,
			eps_vg, dis, mark);
	}
}

template<class T>
void wxn_pruning(const string& DGG_file_name, double eps_vg, string& test_output_filename, int thread_num, double& prune_time)
{
	vector<vector<int>> graph_neighbor;
	vector<vector<T>> graph_neighbor_dis;
	vector<vector<bool>> graph_neighbor_deleted;
	int node_number;

	readInputFile(DGG_file_name, graph_neighbor, graph_neighbor_dis, graph_neighbor_deleted, node_number);

	ElapasedTime t;
	{
		int part_size = node_number / thread_num;
		vector<std::thread> tt(thread_num);
		for (int thread_id = 0; thread_id < thread_num; ++thread_id) {


			tt[thread_id] = std::thread(&dijkstraPruningThread<T>, thread_id,
				thread_num, node_number, ref(graph_neighbor),
				ref(graph_neighbor_dis), ref(graph_neighbor_deleted),
				eps_vg);

		}
		for (int i = 0; i < thread_num; ++i) {
			tt[i].join();
		}

	}
	//fz
	//t.printTime();
	prune_time = t.getTime();
	std::ifstream input_file(DGG_file_name, std::ios::in | std::ios::binary);
	HeadOfDGG head_of_DGG;
	input_file.read((char*)&head_of_DGG, sizeof(head_of_DGG));
	//fz
	//head_of_DGG.print();

	ofstream output_file(test_output_filename, std::ios::out | std::ios::binary);
	output_file.write((char*)&head_of_DGG, sizeof(head_of_DGG));

	int cnt = 0;
	//fz
	int cnt_deleted = 0;
	for (int i = 0; i < head_of_DGG.num_of_vertex; ++i) {
		BodyHeadOfDGG body_head;
		input_file.read((char*)&body_head, sizeof(body_head));
		vector<int> new_graph_neighbor;
		vector<T> new_graph_neighbor_dis;
		vector<int> origin2current(body_head.neighbor_num, -1);
		for (int j = 0; j < body_head.neighbor_num; ++j) {
			if (!graph_neighbor_deleted[i][j]) {
				cnt++;
				origin2current[j] = new_graph_neighbor.size();
				new_graph_neighbor.push_back(graph_neighbor[i][j]);
				new_graph_neighbor_dis.push_back(graph_neighbor_dis[i][j]);
			}
			else {
				cnt_deleted++;
			}
		}
		int start_pos = 0;
		for (int j = 0; j < origin2current.size(); ++j) {
			if (origin2current[j] == -1) {
				origin2current[j] = start_pos;
			}
			else{
				start_pos = origin2current[j];
			}
		}
		vector<BodyPartOfDGGWithAngle> body_parts;
		for (int j = 0; j < body_head.neighbor_num; ++j){
			BodyPartOfDGGWithAngle b;
			input_file.read((char*)&b, sizeof(b));
			body_parts.push_back(b);
		}
		body_head.neighbor_num = new_graph_neighbor.size();
		output_file.write((char*)&body_head, sizeof(body_head));
		for (int j = 0; j < body_parts.size(); ++j) {
			if (!graph_neighbor_deleted[i][j]) {
				BodyPartOfDGGWithAngle b = body_parts[j];
				if (b.begin_pos == -1) {
					b.begin_pos = -1;
					b.end_pos = -1;
				}
				else {
					b.begin_pos = origin2current[b.begin_pos];
					b.end_pos = origin2current[b.end_pos];
				}
				output_file.write((char*)&b, sizeof(b));
			}
		}
	}
	//fz
	//fprintf(stderr, "## Average neighbors Indirect\t%lf\n", (double)cnt / node_number);
	//fprintf(stderr, "## Redundency ratio \t%lf\n", (double) cnt_deleted / (double) (cnt + cnt_deleted));
	fprintf(stderr, "%lf\t", (double)cnt_deleted / (double)(cnt + cnt_deleted));
	input_file.close();
	output_file.close();
}

void DGG_precompute_ich_multithread(const string& input_obj_name, double eps_vg, string& DGG_file_name, double const_for_theta, int thread_num, int method, int fixed_K = 0)
{
	double ich_multi_time;
	DGG_precompute_ich_multithread_before_pruning(input_obj_name, eps_vg, DGG_file_name, const_for_theta, thread_num, ich_multi_time, method, fixed_K);
	string time_file(DGG_file_name.substr(0, DGG_file_name.length() - 7) + ".time");
	FILE *tf = fopen(time_file.c_str(), "w");
	fprintf(tf, "%lf\n", ich_multi_time);
	if (method == 1) {
		string wxn_output_filename(DGG_file_name.substr(0, DGG_file_name.length() - 7) + "_pruning.binary");
		double prune_time;
		wxn_pruning<double>(DGG_file_name, eps_vg, wxn_output_filename, thread_num, prune_time);
		fprintf(tf, "%lf\n", prune_time);
		fprintf(stderr, "## Pruning (seconds) \t%lf\n", prune_time);
		fprintf(stderr, "## Precomputation(+Pruning) (seconds) \t%lf\n", ich_multi_time + prune_time);
	}
	else {
		fprintf(stderr, "## Precomputation (seconds) \t%lf\n", ich_multi_time);
	}
	fclose(tf);
#if 0
	PROCESS_MEMORY_COUNTERS pmc;
	if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
	{
		//cout << "MEM USAGE: " << endl;
		fprintf(stderr, "DGG after pruning mem usage\n");
		cerr << " PeakWSS: " << pmc.PeakWorkingSetSize / 1024 << " KB " << endl;
		cerr << " WSS: " << pmc.WorkingSetSize / 1024 << " KB " << endl;
		cerr << " PFU: " << pmc.PagefileUsage / 1024 << " KB " << endl;
		cerr << " PeakPFU: " << pmc.PeakPagefileUsage / 1024 << " KB " << std::endl;
	}
#endif
}
