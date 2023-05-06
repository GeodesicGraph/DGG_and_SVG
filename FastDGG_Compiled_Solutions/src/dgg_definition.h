#ifndef _DGG_DEFINITION_H_
#define _DGG_DEFINITION_H_
#include <stdint.h>

struct HeadOfDGG {
	int begin_vertex_index;
	int end_vertex_index;
	int num_of_vertex;
	HeadOfDGG(int _begin_vertex_index,
		int _end_vertex_index,
		int _num_of_vertex)
		:
		begin_vertex_index(_begin_vertex_index),
		end_vertex_index(_end_vertex_index),
		num_of_vertex(_num_of_vertex){}
	HeadOfDGG(){}
	void print(){
		fprintf(stderr, "head of DGG : num_of_vertex %d\n", num_of_vertex);
	}
};

struct BodyHeadOfDGG{
	int source_index;
	int neighbor_num;
	BodyHeadOfDGG(int _source_index, int _neighbor_num)
	{
		source_index = _source_index;
		neighbor_num = _neighbor_num;
	}
	BodyHeadOfDGG(){}
};
struct BodyPartOfDGG{
	int dest_index;
	double dest_dis;
	BodyPartOfDGG(){}
	BodyPartOfDGG(int _dest_index, double _dest_dis) :
		dest_index(_dest_index),
		dest_dis(_dest_dis)
	{
		dest_index = _dest_index;
		dest_dis = _dest_dis;
	}
};

struct BodyPartOfDGGWithAngle{
	int dest_index;
	double dest_dis;
	double angle;
	int begin_pos;
	int end_pos;
	BodyPartOfDGGWithAngle(){}
	BodyPartOfDGGWithAngle(int _dest_index, double _dest_dis, double _angle, int _begin_pos, int _end_pos) :
		dest_index(_dest_index),
		dest_dis(_dest_dis),
		angle(_angle),
		begin_pos(_begin_pos),
		end_pos(_end_pos)
	{
	}
	bool operator<(const BodyPartOfDGGWithAngle& o) const{
		return angle < o.angle;
	}
};

#endif
