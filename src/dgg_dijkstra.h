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


template<typename T>  class SparseGraph{
protected:
	std::vector<std::vector<int>> graph_neighbor;
	std::vector<std::vector<T>> graph_neighbor_dis;

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

private:
	void initialize(int _node_number) {
		node_number_ = _node_number;
		graph_neighbor.reserve(_node_number);
		graph_neighbor.resize(_node_number);
		graph_neighbor_dis.reserve(_node_number);
		graph_neighbor_dis.resize(_node_number);
	}

	void allocate_for_neighbor_small(int u, int number_of_neighbor) {
		graph_neighbor[u].reserve(number_of_neighbor);
		//graph_pos_in_neighbor[u].reserve(number_of_neighbor);
		graph_neighbor_dis[u].reserve(number_of_neighbor);
		//graph_neighbor_angle[u].reserve(number_of_neighbor);
		//graph_neighbor_begin_end_pos[u].reserve(number_of_neighbor);
	}

	void addedge(int u, int v, T w) {
		//u , v is the two edge
		// w is the distance
		assert(u < node_number_ && v < node_number_);
		graph_neighbor[u].push_back(v);
		graph_neighbor_dis[u].push_back(w);
	}

public:	
	int NodeNum() {
		return node_number_;
	}

	int readDGGFile(const std::string& DGG_file_name) {

		std::ifstream input_file(DGG_file_name, std::ios::in | std::ios::binary);
		HeadOfDGG head_of_DGG;
		input_file.read((char*)&head_of_DGG, sizeof(head_of_DGG));
		initialize(head_of_DGG.num_of_vertex);

		double average_neighbor_number(0.0);

		printf("Loading Graph...");
		
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
			allocate_for_neighbor_small(body_head.source_index, body_parts.size());
			for (int j = 0; j < body_parts.size(); ++j) {
				addedge(body_head.source_index,
					body_parts[j].dest_index,
					body_parts[j].dest_dis);
			}
		}
		num_edges_ = average_neighbor_number;
		average_neighbor_number /= head_of_DGG.num_of_vertex;
		average_degree_ = average_neighbor_number;

		printf("Loaded Graph:\n\t## Average Degree = %lf\n", average_neighbor_number);
		
		input_file.close();
		return 0;
	}

	virtual void getPath(const int& dest, std::vector<int>& path_nodes) = 0;
	virtual const std::vector<T>* distanceField() = 0;
	virtual T distanceToSource(int index) = 0;
	virtual void findShortestDistance(int source) = 0;
	virtual void findShortestDistance(const std::vector<int>& sources) = 0;
	virtual void findShortestDistance(int source, bool flag_normalize) = 0;
	virtual void findShortestDistance(const std::vector<int>& sources, bool flag_normalize) = 0;
};

template <typename T> class Dijkstra : public SparseGraph<T> {
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
	T max_dis;
	std::vector<T> dis;
	std::vector<bool>visited;
	std::vector<int> fathers;

public:
	Dijkstra(){}

	void getPath(const int& dest, std::vector<int>& path_nodes) override{
		path_nodes.clear();
		int u = dest;
		while (u != fathers[u]){
			path_nodes.push_back(u);
			u = fathers[u];
		}
		path_nodes.push_back(u);
		std::reverse(path_nodes.begin(), path_nodes.end());
	}

	const std::vector<T>* distanceField() override {
		return &dis;
	}

	inline T distanceToSource(int index) {
		if (index < 0 || index >= node_number_) {
			cerr << "wrong index " << index << "\n";
			return 0;
		}
		return dis[index];
	}

	void findShortestDistance(int source) override
	{
		fathers.resize(node_number_);
		fill(fathers.begin(), fathers.end(), -1);
		dis.resize(node_number_);
		fill(dis.begin(), dis.end(), std::numeric_limits<T>::max());
		visited.resize(node_number_);
		fill(visited.begin(), visited.end(), false);
		std::priority_queue<QueueNode> pq;
		dis[source] = 0;
		pq.push(QueueNode(source, 0));
		fathers[source] = source;
		while (!pq.empty()){
			QueueNode u = pq.top();
			pq.pop();
			if (visited[u.node_index]) continue;
			visited[u.node_index] = true;

			for (int i = 0; i < graph_neighbor[u.node_index].size(); ++i){
				int v = graph_neighbor[u.node_index][i];
				T w = graph_neighbor_dis[u.node_index][i];
				if (!visited[v] && dis[v] > dis[u.node_index] + w){
					dis[v] = dis[u.node_index] + w;
					pq.push(QueueNode(v, dis[v]));
					fathers[v] = u.node_index;
				}
			}
		}
	}

	void findShortestDistance(const std::vector<int>& sources) override
	{
		fathers.resize(node_number_);
		fill(fathers.begin(), fathers.end(), -1);
		dis.resize(node_number_);
		fill(dis.begin(), dis.end(), std::numeric_limits<T>::max());
		visited.resize(node_number_);
		fill(visited.begin(), visited.end(), false);
		std::priority_queue<QueueNode> pq;
		for (auto& source : sources) {
			dis[source] = 0;
			pq.push(QueueNode(source, 0));
			fathers[source] = source;
		}

		while (!pq.empty()) {
			QueueNode u = pq.top();
			pq.pop();
			if (visited[u.node_index]) continue;
			visited[u.node_index] = true;

			for (int i = 0; i < graph_neighbor[u.node_index].size(); ++i) {
				int v = graph_neighbor[u.node_index][i];
				T w = graph_neighbor_dis[u.node_index][i];
				if (!visited[v] && dis[v] > dis[u.node_index] + w) {
					dis[v] = dis[u.node_index] + w;
					pq.push(QueueNode(v, dis[v]));
					fathers[v] = u.node_index;
				}
			}
		}
	}

	void normalizeDistanceField() {
		max_dis = -1.0;
		for (int i = 0; i < node_number_; ++i) {
			if (dis[i] > 1e10) continue;
			if (!_finite(dis[i])) continue;
			max_dis = max(max_dis, dis[i]);
		}
		for (int i = 0; i < node_number_; ++i) {
			if (dis[i] > 1e10) dis[i] = -1;
			else dis[i] /= max_dis;
		}
	}

	void findShortestDistance(int source, bool flag_normalize) override
	{
		findShortestDistance(source);
		if (flag_normalize) {
			normalizeDistanceField();
		}
	}

	void findShortestDistance(const std::vector<int>& sources, bool flag_normalize) override
	{
		findShortestDistance(sources);
		if (flag_normalize) {
			normalizeDistanceField();
		}
	}
};

#endif