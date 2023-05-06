#ifndef _DGG_DIJKSTRA_H_
#define _DGG_DIJKSTRA_H_

#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <algorithm>
#include <list>
#include <map>
#include <fstream>
#include <assert.h>

#include "DGG_definition.h"
#include "ICH\RichModel.h"

template<typename T>  class SparseGraph{
public:
	std::vector<std::vector<int>> graph_neighbor;
	std::vector<std::vector<int16_t>> graph_pos_in_neighbor;
	std::vector<std::vector<T>> graph_neighbor_dis;
	std::vector<std::vector<pair<int16_t, int16_t>>> graph_neighbor_begin_end_pos;
	std::vector<std::map<int, int16_t>> graph_neighbor_map;
	std::vector<std::vector<T>> graph_neighbor_angle;

public:
	int node_number_;
	int num_edges_;
	double average_degree_;

public:
	SparseGraph(){
		node_number_ = -1;
	}

	const std::vector<int>& graphNeighbor(int v) {
		return graph_neighbor[v];
	}
	const std::vector<T>& graphNeighborDis(int v) {
		return graph_neighbor_dis[v];
	}
	const std::vector<T>& graphNeighborAngle(int v) {
		return graph_neighbor_angle[v];
	}

private:
	void initialize(int _node_number) {
		node_number_ = _node_number;
		graph_neighbor.reserve(_node_number);
		graph_neighbor.resize(_node_number);
		graph_pos_in_neighbor.reserve(_node_number);
		graph_pos_in_neighbor.resize(_node_number);
		graph_neighbor_dis.reserve(_node_number);
		graph_neighbor_dis.resize(_node_number);
		graph_neighbor_angle.reserve(_node_number);
		graph_neighbor_angle.resize(_node_number);
		graph_neighbor_begin_end_pos.reserve(_node_number);
		graph_neighbor_begin_end_pos.resize(_node_number);
		graph_neighbor_map.reserve(_node_number);
		graph_neighbor_map.resize(_node_number);
	}

	void allocate_for_neighbor_small(int u, int number_of_neighbor) {
		graph_neighbor[u].reserve(number_of_neighbor);
		//graph_pos_in_neighbor[u].reserve(number_of_neighbor);
		graph_neighbor_dis[u].reserve(number_of_neighbor);
		//graph_neighbor_angle[u].reserve(number_of_neighbor);
		//graph_neighbor_begin_end_pos[u].reserve(number_of_neighbor);
	}
	void allocate_for_neighbor_with_angle(int u, int number_of_neighbor) {
		graph_neighbor[u].reserve(number_of_neighbor);
		graph_pos_in_neighbor[u].reserve(number_of_neighbor);
		graph_neighbor_dis[u].reserve(number_of_neighbor);
		graph_neighbor_angle[u].reserve(number_of_neighbor);
		graph_neighbor_begin_end_pos[u].reserve(number_of_neighbor);
	}
	void allocate_for_neighbor_with_range(int u, int number_of_neighbor) {
		graph_neighbor[u].reserve(number_of_neighbor);
		graph_pos_in_neighbor[u].reserve(number_of_neighbor);
		graph_neighbor_dis[u].reserve(number_of_neighbor);
		graph_neighbor_begin_end_pos[u].reserve(number_of_neighbor);
	}

	void addedge(int u, int v, T w) {
		//u , v is the two edge
		// w is the distance
		assert(u < node_number_ && v < node_number_);
		graph_neighbor[u].push_back(v);
		graph_neighbor_dis[u].push_back(w);
	}

public:
	void addedge(int u, int v, T w, T angle, int begin_pos, int end_pos) {
		//u , v is the two edge
		// w is the distance
		assert(u < node_number_ && v < node_number_);
		graph_neighbor[u].push_back(v);
		graph_neighbor_dis[u].push_back(w);
		graph_neighbor_map[u][v] = graph_neighbor_angle[u].size();
		graph_neighbor_angle[u].push_back(angle);
		graph_neighbor_begin_end_pos[u].push_back(make_pair(begin_pos, end_pos));
	}
	
	int NodeNum() {
		return node_number_;
	}

	int readDGGFileNotLoadAngle(const std::string& DGG_file_name) {

		std::ifstream input_file(DGG_file_name, std::ios::in | std::ios::binary);
		std::cout << "start loading\n" << std::endl;
		HeadOfDGG head_of_DGG;
		input_file.read((char*)&head_of_DGG, sizeof(head_of_DGG));
		//head_of_DGG.print();
		initialize(head_of_DGG.num_of_vertex);

		double average_neighbor_number(0.0);
		double max_radius_in_file(0.0);
		double average_radius(0.0);

		for (int i = 0; i < head_of_DGG.num_of_vertex; ++i) {
			//printf("i %d %d", i, head_of_DGG.num_of_vertex);
			BodyHeadOfDGG body_head;
			input_file.read((char*)&body_head, sizeof(body_head));
			average_neighbor_number += (double)body_head.neighbor_num;
			std::vector<BodyPartOfDGGWithAngle> body_parts;
			for (int j = 0; j < body_head.neighbor_num; ++j){
				BodyPartOfDGGWithAngle body_part;
				input_file.read((char*)&body_part, sizeof(body_part));
				body_parts.push_back(body_part);
			}
			allocate_for_neighbor_small(body_head.source_index, body_parts.size());
			for (int j = 0; j < body_parts.size(); ++j) {
				addedge(body_head.source_index,
					body_parts[j].dest_index,
					body_parts[j].dest_dis);
			}
			if (i > 0 && i % (head_of_DGG.num_of_vertex / 10) == 0){
				//std::cerr << "read " << i * 100 / head_of_DGG.num_of_vertex << " percent \n";
			}
		}

		num_edges_ = average_neighbor_number;
		average_neighbor_number /= head_of_DGG.num_of_vertex;
		average_degree_ = average_neighbor_number;
		fprintf(stderr, "average_neigh %lf\n", average_neighbor_number);
		printf("%lf\n", average_neighbor_number);
		//std::cerr << "reading done..\n";
		input_file.close();
		return 0;
	}

	int read_DGG_file_with_angle(const std::string& DGG_file_name) {

		std::ifstream input_file(DGG_file_name, std::ios::in | std::ios::binary);
		HeadOfDGG head_of_DGG;
		input_file.read((char*)&head_of_DGG, sizeof(head_of_DGG));
		//fz
		//head_of_DGG.print();
		initialize(head_of_DGG.num_of_vertex);

		double average_neighbor_number(0.0);
		double max_radius_in_file(0.0);
		double average_radius(0.0);

		for (int i = 0; i < head_of_DGG.num_of_vertex; ++i) {
			BodyHeadOfDGG body_head;
			input_file.read((char*)&body_head, sizeof(body_head));
			average_neighbor_number += (double)body_head.neighbor_num;
			std::vector<BodyPartOfDGGWithAngle> body_parts;
			for (int j = 0; j < body_head.neighbor_num; ++j){
				BodyPartOfDGGWithAngle body_part;
				input_file.read((char*)&body_part, sizeof(body_part));
				body_parts.push_back(body_part);
			}
			allocate_for_neighbor_with_angle(body_head.source_index, body_parts.size());
			for (int j = 0; j < body_parts.size(); ++j) {
				addedge(body_head.source_index,
					body_parts[j].dest_index,
					body_parts[j].dest_dis,
					body_parts[j].angle,
					body_parts[j].begin_pos,
					body_parts[j].end_pos
					);
			}
			if (i > 0 && i % (head_of_DGG.num_of_vertex / 10) == 0){
				//fz
				//std::cerr << "read " << i * 100 / head_of_DGG.num_of_vertex  << " percent \n";
			}
		}

		num_edges_ = average_neighbor_number;
		average_neighbor_number /= head_of_DGG.num_of_vertex;
		average_degree_ = average_neighbor_number;
		//fz
		fprintf(stderr, "average_neigh %lf\n", average_neighbor_number);
		printf("%lf\n", average_neighbor_number);
		/*std::cerr << "reading done..\n";*/
		input_file.close();

		for (int i = 0; i < graph_pos_in_neighbor.size(); ++i) {
			graph_pos_in_neighbor[i].resize(graph_neighbor[i].size());
			for (int j = 0; j < graph_neighbor[i].size(); ++j) {
				int neigh = graph_neighbor[i][j];
				int pos = graph_neighbor_map[neigh][i];
				graph_pos_in_neighbor[i][j] = pos;
			}
		}

		return 0;
	}

	virtual void findShortestDistance(int source) = 0;
	//virtual int getSource(int v)=0;
	virtual T distanceToSource(int index) = 0;
	virtual void getPath(const int& dest, std::vector<int>& path_nodes) = 0;
	virtual void findShortestDistance(int source, bool flag_normalize) = 0;
	virtual void findShortestDistance(const vector<int>& sources, bool flag_normalize) = 0;
	virtual const vector<T>* distanceField() = 0;
};




template <typename T> class LC_LLL :public SparseGraph<T>{

private:
	T max_dis;
	std::vector<T> dis;
	std::vector<bool> visited;
	std::vector<int> fathers;

public:
	LC_LLL(){}

	void getPath(const int& dest, std::vector<int>& path_nodes) {
		path_nodes.clear();
		int u = dest;
		while (u != fathers[u]){
			path_nodes.push_back(u);
			u = fathers[u];
		}
		path_nodes.push_back(u);
		std::reverse(path_nodes.begin(), path_nodes.end());
	}

	const vector<T>* distanceField() {
		return &dis;
	}
	
	void normalizeDistanceField() {
		//T max_dis = 0.0;
		max_dis = 0.0;
		for (int i = 0; i < node_number_; ++i) {
			max_dis = max(max_dis, dis[i]);
		}
		//max_dis = 1.0 / max_dis;
		for (int i = 0; i < node_number_; ++i) {
			dis[i] /= max_dis;
		}
	} 

	void findShortestDistance(int source, bool flag_normalize)
	{
		findShortestDistance(source);
		if (flag_normalize) {
			normalizeDistanceField();
		}
	}

	void findShortestDistance(const vector<int>& sources, bool flag_normalize)
	{

		findShortestDistance(sources);
		if (flag_normalize) {
			normalizeDistanceField();
		}
	}

	void findShortestDistance(const vector<int>& sources)
	{
		fathers.resize(node_number_);
		fill(fathers.begin(), fathers.end(), -1);
		dis.resize(node_number_);
		fill(dis.begin(), dis.end(), FLT_MAX);
		visited.resize(node_number_);
		fill(visited.begin(), visited.end(), false);
		std::deque<int> que;
		double dis_sum = 0.0;
		for (auto& source : sources) {
			dis[source] = 0;
			que.push_back(source);
			fathers[source] = source;
			visited[source] = true;
		}

		int processed_ele_ment = 0;
		int total_neigh = 0;
		while (!que.empty()) {
			int u = -1;
			processed_ele_ment++;
			while (true) {

				if (dis[que.front()] > dis_sum / que.size()) {
					que.push_back(que.front());
					que.pop_front();
				}
				else {
					u = que.front();
					que.pop_front();
					dis_sum -= dis[u];
					visited[u] = false;
					break;
				}
			}
			for (int i = 0; i < graph_neighbor[u].size(); ++i) {
				if (dis[graph_neighbor[u][i]] > dis[u] + graph_neighbor_dis[u][i]) {
					total_neigh++;
					int v = graph_neighbor[u][i];
					T w = graph_neighbor_dis[u][i];
					double old_v = dis[v];
					dis[v] = dis[u] + w;
					fathers[v] = u;
					if (visited[v] == false) {
						que.push_back(v);
						visited[v] = true;
						dis_sum += dis[v];
					}
					else {
						dis_sum -= old_v;
						dis_sum += dis[v];
					}
				}
			}
		}
		//printf("********* processed_element %d total_vert %d percenter %.2lf%%\n" , processed_ele_ment, node_number_, processed_ele_ment / (double) node_number_ * 100.0); 
		//printf("********* total_neigh %d total_vert %d percenter %.2lf%%\n" , total_neigh, node_number_, total_neigh / (double) node_number_ * 100.0); 


	}

	void findShortestDistance(int source)
	{
		fathers.resize(node_number_);
		fill(fathers.begin(), fathers.end(), -1);
		dis.resize(node_number_);
		fill(dis.begin(), dis.end(), FLT_MAX);
		visited.resize(node_number_);
		fill(visited.begin(), visited.end(), false);
		std::deque<int> que;
		double dis_sum = 0.0;
		dis[source] = 0;
		que.push_back(source);
		fathers[source] = source;
		visited[source] = true;

		int processed_ele_ment = 0;
		int total_neigh = 0;
		while (!que.empty()) {
			int u = -1;
			processed_ele_ment++;
			while (true) {

				if (dis[que.front()] > dis_sum / que.size()) {
					que.push_back(que.front());
					que.pop_front();
				}
				else {
					u = que.front();
					que.pop_front();
					dis_sum -= dis[u];
					visited[u] = false;
					break;
				}
			}
			for (int i = 0; i < graph_neighbor[u].size(); ++i) {
				if (dis[graph_neighbor[u][i]] > dis[u] + graph_neighbor_dis[u][i]) {
					total_neigh++;
					int v = graph_neighbor[u][i];
					T w = graph_neighbor_dis[u][i];
					double old_v = dis[v];
					dis[v] = dis[u] + w;
					fathers[v] = u;
					if (visited[v] == false) {
						que.push_back(v);
						visited[v] = true;
						dis_sum += dis[v];
					}
					else {
						dis_sum -= old_v;
						dis_sum += dis[v];
					}
				}
			}
		}
		//fz
		//fprintf(stderr,"********* processed_element %d total_vert %d percenter %.2lf%%\n" , processed_ele_ment, node_number_, processed_ele_ment / (double) node_number_ * 100.0); 
		//fprintf(stderr,"********* total_neigh %d total_vert %d percenter %.2lf%%\n" , total_neigh, node_number_, total_neigh / (double) node_number_ * 100.0); 


	}

	inline int getSource(int v){
		if (fathers[v] == v){
			return v;
		}
		else{
			fathers[v] = getSource(fathers[v]);
			return fathers[v];
		}
	}

	inline T distanceToSource(int index) {
		if (index < 0 || index >= node_number_){
			std::cerr << "wrong index " << index << "\n";
			return 0;
		}
		return dis[index];
	}

};



template <typename T> class  Dijstra_vector :public SparseGraph<T> {
public:
private:
	struct QueueNode{
		T dis;
		int node_index;
		QueueNode(){}
		QueueNode(int _node_index, T _dis){
			dis = _dis;
			node_index = _node_index;
		}
		bool operator<(const QueueNode& other)const{
			return dis > other.dis;
		}
	};
	std::vector<T> dis;
	std::vector<bool>visited;
	std::vector<int> fathers;

public:
	Dijstra_vector(){}

	void getPath(const int& dest, std::vector<int>& path_nodes){
		path_nodes.clear();
		int u = dest;
		while (u != fathers[u]){
			path_nodes.push_back(u);
			u = fathers[u];
		}
		path_nodes.push_back(u);
		std::reverse(path_nodes.begin(), path_nodes.end());
	}

	const vector<T>* distanceField() {
		return &dis;
	}

	void findShortestDistance(int source)
	{
		fathers.resize(node_number_);
		fill(fathers.begin(), fathers.end(), -1);
		dis.resize(node_number_);
		fill(dis.begin(), dis.end(), FLT_MAX);
		visited.resize(node_number_);
		fill(visited.begin(), visited.end(), false);
		priority_queue<QueueNode> que;
		dis[source] = 0;
		que.push(QueueNode(source, 0));
		fathers[source] = source;
		int max_que_size = 0;

		while (!que.empty()){
			QueueNode u = que.top();
			max_que_size = max((int)que.size(), max_que_size);
			que.pop();
			if (visited[u.node_index]) continue;
			visited[u.node_index] = true;

			for (int i = 0; i < graph_neighbor[u.node_index].size(); ++i){
				int v = graph_neighbor[u.node_index][i];
				T w = graph_neighbor_dis[u.node_index][i];
				if (!visited[v] && dis[v] > dis[u.node_index] + w){
					dis[v] = dis[u.node_index] + w;
					que.push(QueueNode(v, dis[v]));
					fathers[v] = u.node_index;
				}
			}

		}
		//printf("max queue size %d\n" , max_que_size );
	}

	int getSource(int v){
		if (fathers[v] == v){
			return v;
		}
		else{
			fathers[v] = getSource(fathers[v]);
			return fathers[v];
		}
	}



	inline T distanceToSource(int index){
		if (index < 0 || index >= node_number_){
			cerr << "wrong index " << index << "\n";
			return 0;
		}
		return dis[index];
	}


};


#endif