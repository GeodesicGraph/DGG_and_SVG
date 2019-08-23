#include "stdafx.h"
#include <windows.h>
#include "Shlwapi.h"
#include <thread>
#include <random>
#include <conio.h>

#include "dgg_definition.h"
#include "dgg_time.h"
#include "dgg_buffer.h"
#include "dgg_precompute.h"
#include "ICH\RichModel.h"
#include "SVG.h"
#include "FastDGG.h"

using namespace std;

void getDisAndAngles(const CRichModel& model, int source, double eps_vg, vector<int>& dests, vector<double>& angles, vector<double>& dis, int method, int fixed_K)
{
	if (method == 0){
		CFastDGG alg(model, vector < int > {source});
		set<int> fixed_dests;
		alg.ExecuteLocally_FastDGG(eps_vg, fixed_dests);
		dests.clear();
		dests.reserve(fixed_dests.size());
		dis.reserve(fixed_dests.size());
		for (auto d : fixed_dests) {
			map<int, CFastDGG::InfoAtVertex>::const_iterator it = alg.m_InfoAtVertices.find(d);
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
	else if (method == 1) {
		CSVG alg(model, vector < int > {source});
		set<int> fixed_dests;
		double max_radius = std::numeric_limits<double>::max();
		if (method == 1) alg.ExecuteLocally_SVG(max_radius, fixed_dests, fixed_K);
		dests.clear();
		dests.reserve(fixed_dests.size());
		dis.reserve(fixed_dests.size());
		for (auto d : fixed_dests) {
			map<int, CSVG::InfoAtVertex>::const_iterator it = alg.m_InfoAtVertices.find(d);
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
	double theta, int source,
	vector<BodyPartOfDGGWithAngle>& body_parts_with_angle)
{

	body_parts_with_angle.reserve(dests.size());
	for (int i = 0; i < dests.size(); ++i) {
		BodyPartOfDGGWithAngle b_with_angle(dests[i], dis[i], angles[i], -1, -1);
		body_parts_with_angle.push_back(b_with_angle);
	}
}

void ichPropogateHead(const HeadOfDGG& head, const string& part_dgg_filename, double eps_vg, double theta, const CRichModel& model, int thread_id, int method, int fixed_K, long long& thread_degree)
{
	DGGBuffer dgg_buffer;
	dgg_buffer.open(part_dgg_filename);
	dgg_buffer.addStruct(&head, sizeof(head));
	ElapasedTime time_once;

	thread_degree = 0;
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
		thread_degree += body_parts_with_angle.size();
	}
	dgg_buffer.close();

}

void combinePartPrecomputeFiles(const vector<string>& part_filenames, const string& dgg_filename, int num_of_vertex, int thread_num)
{
	HeadOfDGG head;
	head.begin_vertex_index = 0; head.end_vertex_index = num_of_vertex - 1;
	head.num_of_vertex = num_of_vertex;
	ofstream output_file(dgg_filename.c_str(), ios::out | ios::binary);
	output_file.write((char*)&head, sizeof(head));

	for (int thread_id = 0; thread_id < thread_num; ++thread_id) {
		const string& part_dgg_filename = part_filenames[thread_id];
		std::ifstream input_file(part_dgg_filename, std::ios::in | std::ios::binary);
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

void DGG_ICH_Precompute_Multithread(const std::string& input_obj_name, double accuracy_control_parameter,
	double const_for_theta, int thread_num)
{
	char buf[1024];
	sprintf(buf, "%s_FD%.10lf_c%.0lf.binary", input_obj_name.substr(0, input_obj_name.length() - 4).c_str(), accuracy_control_parameter, const_for_theta);
	string dgg_file_name = string(buf);
	double theta = asin(sqrt(accuracy_control_parameter)) * const_for_theta;
	
	CRichModel model(input_obj_name);
	model.Preprocess();

	vector<HeadOfDGG> heads;
	vector<string> graph_part_file_names;
	int part_size = model.GetNumOfVerts() / thread_num;
	for (int i = 0; i < thread_num; ++i) {
		HeadOfDGG head;
		head.num_of_vertex = model.GetNumOfVerts();
		head.begin_vertex_index = i * part_size;
		if (i != thread_num - 1) {
			head.end_vertex_index = (i + 1)*part_size - 1;
		}
		else {
			head.end_vertex_index = model.GetNumOfVerts() - 1;
		}
		heads.push_back(head);
		char buf[1024];
		sprintf(buf, "%s_FD%.10lf_c%.0lf_part%d.binary", input_obj_name.substr(0, input_obj_name.length() - 4).c_str(), accuracy_control_parameter, const_for_theta, i);
		graph_part_file_names.push_back((string)buf);
	}

	vector<std::thread> tt(thread_num);
	//long long degree = 0;
	vector<long long> thread_degree(thread_num);
	for (int i = 0; i < thread_num; ++i) {
		tt[i] = std::thread(&ichPropogateHead, heads[i], graph_part_file_names[i], accuracy_control_parameter, theta, std::ref(model), thread_num - i, 0, 0, std::ref(thread_degree[i]));
	}
	for (int i = 0; i < thread_num; ++i) {
		tt[i].join();
	}
	//for (int i = 0; i < thread_num; ++i)
	//	degree += thread_degree[i];

	combinePartPrecomputeFiles(graph_part_file_names, dgg_file_name, model.GetNumOfVerts(), thread_num);
}

void SVG_ICH_Precompute_Multithread(const std::string& input_obj_name, int fixed_K, int thread_num)
{
	char buf[1024];
	sprintf(buf, "%s_SVG_K%d.binary", input_obj_name.substr(0, input_obj_name.length() - 4).c_str(), fixed_K);
	string dgg_file_name = string(buf);
	
	CRichModel model(input_obj_name);
	model.Preprocess();

	vector<HeadOfDGG> heads;
	vector<string> graph_part_file_names;
	int part_size = model.GetNumOfVerts() / thread_num;
	for (int i = 0; i < thread_num; ++i) {
		HeadOfDGG head;
		head.num_of_vertex = model.GetNumOfVerts();
		head.begin_vertex_index = i * part_size;
		if (i != thread_num - 1) {
			head.end_vertex_index = (i + 1)*part_size - 1;
		}
		else {
			head.end_vertex_index = model.GetNumOfVerts() - 1;
		}
		heads.push_back(head);
		char buf[1024];
		sprintf(buf, "%s_SVG_K%d_part%d.binary", input_obj_name.substr(0, input_obj_name.length() - 4).c_str(), fixed_K, i);
		graph_part_file_names.push_back((string)buf);
	}

	vector<std::thread> tt(thread_num);
	//long long degree = 0;
	vector<long long> thread_degree(thread_num);
	for (int i = 0; i < thread_num; ++i) {
		tt[i] = std::thread(&ichPropogateHead, heads[i], graph_part_file_names[i], 0, 0, std::ref(model), thread_num - i, 1, fixed_K, std::ref(thread_degree[i]));
	}
	for (int i = 0; i < thread_num; ++i) {
		tt[i].join();
	}
	/*for (int i = 0; i < thread_num; ++i)
		degree += thread_degree[i];*/

	combinePartPrecomputeFiles(graph_part_file_names, dgg_file_name, model.GetNumOfVerts(), thread_num);
}
