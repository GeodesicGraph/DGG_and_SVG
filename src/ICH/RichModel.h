// RichModel.h: interface for the CRichModel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RICHOBJMODEL_H__EB74D2F8_BA58_480E_9050_6FBC1C83D98B__INCLUDED_)
#define AFX_RICHOBJMODEL_H__EB74D2F8_BA58_480E_9050_6FBC1C83D98B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "Point3D.h"
#include "BaseModel.h"
#include <algorithm>

class CRichModel : virtual public CBaseModel 
{
public:
	struct CEdge
	{
		int indexOfLeftVert;
		int indexOfRightVert;
		int indexOfOppositeVert;
		int indexOfLeftEdge;
		int indexOfRightEdge;
		int indexOfReverseEdge;
		int indexOfFrontFace;
		int indexOfSimpleEdge;
		double length;
		double xOfPlanarCoordOfOppositeVert;
		double yOfPlanarCoordOfOppositeVert;

		CEdge()
		{		
			indexOfOppositeVert = -1;	//key	
			indexOfLeftEdge = -1;
			indexOfRightEdge = -1;
		}
	};
	struct CSimpleEdge
	{
		int v1,v2;//index of two verts ,and make sure v1 < v2.
		
		CSimpleEdge(){}
		CSimpleEdge(int _v1,int _v2){
			if( _v1 < _v2 ){
				v1 = _v1;v2 = _v2;
			}else if( _v1 > _v2 ){
				v2 = _v1;v1 = _v2;
			}else{
				printf( "error v1 == v2 ,v1:%d,v2%d\n" , _v1,_v2);
			}
		}
		inline bool operator<(const CSimpleEdge& edge)const{
			if( v1 != edge.v1 ){
				return v1 < edge.v1;
			}else {
				return v2 < edge.v2;
			}
		}
		inline bool operator==(const CSimpleEdge& edge) const{
			return v1 == edge.v1 && v2 == edge.v2;
		}
	};

protected:
	void CreateEdgesFromVertsAndFaces();
	void CollectAndArrangeNeighs();
	void ComputeAnglesAroundVerts();
	void ComputePlanarCoordsOfIncidentVertForEdges();
	void ComputeNumOfHoles();
	void ComputeMaxEdgeLength();
	void ComputeSimpleEdge();
public:
	//void LoadBunny();
	
	CRichModel(const std::string &filename);
	void Preprocess();	

	inline int GetSubindexToVert(int root, int neigh) const;
	inline const CEdge& Edge(int edgeIndex) const;	
	inline const CSimpleEdge& SimpleEdge(int simpleEdgeIndex)const;  
	inline const std::vector< std::pair<int, double> >& Neigh(int root) const;
	inline void PointOnFaceNeigh(int face_index, const CPoint3D& p, std::vector<int>& neigh_verts, std::vector<double>& neigh_angles) const;


  inline const  std::vector<double>& NeighAngleSum(int root) const;
  inline int IncidentVertex(int edgeIndex) const;
	inline double AngleSum(int vertIndex) const;
	inline double Curvature(int vertIndex) const;
	inline double ProportionOnEdgeByImage(int edgeIndex, double x, double y) const;
	inline double ProportionOnEdgeByImageAndPropOnLeftEdge(int edgeIndex, double x, double y, double proportion) const;
	inline double ProportionOnEdgeByImageAndPropOnRightEdge(int edgeIndex, double x, double y, double proportion) const;
	inline double ProportionOnLeftEdgeByImage(int edgeIndex, double x, double y, double proportion) const;
	inline double ProportionOnRightEdgeByImage(int edgeIndex, double x, double y, double proportion) const;
	inline double ProportionOnEdgeByImage(int edgeIndex, double x1, double y1, double x2, double y2) const;
	inline void GetPointByRotatingAround(int edgeIndex, double leftLen, double rightLen, double &xNew, double &yNew) const;
	inline void GetPointByRotatingAroundLeftChildEdge(int edgeIndex, double x, double y, double &xNew, double &yNew) const;	
	inline void GetPointByRotatingAroundRightChildEdge(int edgeIndex, double leftLen, double rightLen, double &xNew, double &yNew) const;	

	double DistanceToIncidentAngle(int edgeIndex, double x, double y) const;


	inline int GetNumOfEdges() const;
	inline int GetNumOfSimpleEdges() const;
	inline int GetNumOfValidDirectedEdges() const;
	inline int GetNumOfTotalUndirectedEdges() const;
	inline int GetNumOfGenera() const;
	inline int GetNumOfIsolated() const;
	inline int GetNumOfComponents() const;
	inline int GetNumOfBoundries() const;
	inline double GetMaxEdgeLength() const;
    inline double GetMeanEdgeLength() const;
	inline bool IsConvexVert(int index) const;
	inline bool isBoundaryVert(int index) const;
	inline bool IsClosedModel() const;
	inline bool IsExtremeEdge(int edgeIndex) const;
	inline bool IsStartEdge(int edgeIndex) const;	
	inline bool HasBeenProcessed() const;
	inline int GetFirstEdgeIndex(int faceIndex) const;
	inline int GetSecondEdgeIndex(int faceIndex) const;
	inline int GetThirdEdgeIndex(int faceIndex) const;
	inline int GetEdgeIndexFromFace(int faceIndex, int subIndex) const;
	inline const CEdge& GetEdgeFromFace(int faceIndex, int subIndex) const;
	inline int GetNeighborFaceIndexFromFace(int face_index,int sub_index)const;  
	inline int GetNeighborFaceIndexFromEdge(int edge_index)const;  
	inline int GetEdgeIndexFromTwoVertices(int leftVert, int rightVert) const;//addd by dgg
	inline int GetFaceIndexFromTreeVertices(int v0, int v1, int v2) const;//addd by dgg
	inline CPoint3D ComputeShiftPoint(int indexOfVert) const;
	inline CPoint3D ComputeShiftPoint(int indexOfVert, double factor) const;
	inline CPoint3D ComputeShiftPoint(const CPoint3D& pt, int simpleEdgeId,double factor) const;
	inline CPoint3D ComputeShiftPoint(const CPoint3D& pt, const CFace& face,double factor) const;
    //inline CPoint3D ComputeShiftPoint(const CPoint3D& pt, const int& face_id , double factor) const;
	static CPoint3D CombinePointAndNormalTo(const CPoint3D& pt, const CPoint3D& normal);
	static CPoint3D CombineTwoNormalsTo(const CPoint3D& pt1, double coef1, const CPoint3D& pt2, double coef2);	
	double GetGaussCurvature(int vertIndex) const;
	inline double GetEdgeVariance() const;

	bool fBePreprocessed;
protected:
	bool fLocked;
	int m_nHoles;
	int m_nIsolatedVerts;
	std::vector< std::vector< std::pair<int, double> > > m_NeighsAndAngles;
	std::vector< std::vector<double>> m_neighAngleSum;
	std::vector<bool> m_FlagsForCheckingConvexVerts;
	std::vector<double> m_angle_sum;
	std::vector<CEdge> m_Edges;
	std::vector<CSimpleEdge> m_SimpleEdges;
	double maxEdgeLength;
    double meanEdgeLength;
};


int CRichModel::IncidentVertex(int edgeIndex) const
{
	return Edge(edgeIndex).indexOfOppositeVert;
}

int CRichModel::GetNumOfValidDirectedEdges() const
{
	return (int)m_Faces.size() * 3;
}

int CRichModel::GetNumOfTotalUndirectedEdges() const
{
	return (int)m_Edges.size() / 2;
}

int CRichModel::GetNumOfGenera() const
{
	return int(GetNumOfTotalUndirectedEdges() - (GetNumOfVerts() - m_nIsolatedVerts) - GetNumOfFaces() - GetNumOfBoundries()) / 2 + 1;
}

int CRichModel::GetNumOfComponents() const
{
	return int(GetNumOfVerts() - m_nIsolatedVerts + GetNumOfFaces() + GetNumOfBoundries() - GetNumOfTotalUndirectedEdges()) / 2; 
}

int CRichModel::GetNumOfBoundries() const
{
	return m_nHoles;
}

bool CRichModel::IsClosedModel() const
{
	return GetNumOfValidDirectedEdges() ==  GetNumOfEdges();
}

int CRichModel::GetNumOfIsolated() const
{
	return m_nIsolatedVerts;
}

int CRichModel::GetNumOfEdges() const
{
	return (int)m_Edges.size();
}

int CRichModel::GetNumOfSimpleEdges() const
{
	return (int)m_SimpleEdges.size();
}


bool CRichModel::isBoundaryVert(int index) const
{
	return IsStartEdge(Neigh(index).front().first);
}

inline bool CRichModel::IsConvexVert(int index) const
{
	return m_FlagsForCheckingConvexVerts[index];
}

bool CRichModel::IsExtremeEdge(int edgeIndex) const
{
	return Edge(edgeIndex).indexOfOppositeVert == -1;
}

bool CRichModel::IsStartEdge(int edgeIndex) const
{
	return Edge(Edge(edgeIndex).indexOfReverseEdge).indexOfOppositeVert == -1;
}

const CRichModel::CEdge& CRichModel::Edge(int edgeIndex) const
{
	return m_Edges[edgeIndex];
}

const CRichModel::CSimpleEdge& CRichModel::SimpleEdge(int simpleEdgeIndex) const  
{
	return m_SimpleEdges[simpleEdgeIndex];

}

const std::vector<double>& CRichModel::NeighAngleSum(int root) const
{
  return m_neighAngleSum[root];
}

const std::vector< std::pair<int, double> >& CRichModel::Neigh(int root) const
{
	return m_NeighsAndAngles[root];
}

void CRichModel::PointOnFaceNeigh(int face_index, const CPoint3D& p, std::vector<int>& neigh_verts, std::vector<double>& neigh_angles) const
{
	neigh_verts.clear();
	neigh_angles.clear();
	
	double len_p_to_vert[3];
	auto& f = Face(face_index);
	for (int i = 0; i < 3; ++i) {
		len_p_to_vert[i] = (p - Vert(f[i])).Len();
	}
	for (int i = 0; i < 3; ++i) {
		double l = len_p_to_vert[i];
		double r = len_p_to_vert[(i + 1) % 3];
		double b = (Vert(f[i]) - Vert(f[(i + 1) % 3])).Len();
		double angle = acos((l * l + r * r - b * b) / (2 * l * r));
		neigh_verts.push_back(f[i]);
		neigh_angles.push_back(angle);
	}
}


double CRichModel::ProportionOnEdgeByImageAndPropOnLeftEdge(int edgeIndex, double x, double y, double proportion) const
{
	double x1 = Edge(edgeIndex).xOfPlanarCoordOfOppositeVert * proportion;
	double y1 = Edge(edgeIndex).yOfPlanarCoordOfOppositeVert * proportion;
	return ProportionOnEdgeByImage(edgeIndex, x1, y1, x, y);
}

double CRichModel::ProportionOnEdgeByImageAndPropOnRightEdge(int edgeIndex, double x, double y, double proportion) const
{
	double x1 = Edge(edgeIndex).xOfPlanarCoordOfOppositeVert * (1 - proportion)
		+ Edge(edgeIndex).length * proportion;
	double y1 = Edge(edgeIndex).yOfPlanarCoordOfOppositeVert * (1 - proportion);
	return ProportionOnEdgeByImage(edgeIndex, x1, y1, x, y);
}

double CRichModel::ProportionOnEdgeByImage(int edgeIndex, double x, double y) const
{
	double res = Edge(edgeIndex).xOfPlanarCoordOfOppositeVert * y - Edge(edgeIndex).yOfPlanarCoordOfOppositeVert * x;
	return res / ((y - Edge(edgeIndex).yOfPlanarCoordOfOppositeVert) * Edge(edgeIndex).length);
}

double CRichModel::ProportionOnEdgeByImage(int edgeIndex, double x1, double y1, double x2, double y2) const
{
	double res = x1 * y2 - x2 * y1;
	return res / ((y2 - y1) * Edge(edgeIndex).length);
}

double CRichModel::ProportionOnLeftEdgeByImage(int edgeIndex, double x, double y, double proportion) const
{
	double xBalance = proportion * Edge(edgeIndex).length;
	double res = Edge(edgeIndex).xOfPlanarCoordOfOppositeVert * y - Edge(edgeIndex).yOfPlanarCoordOfOppositeVert * (x - xBalance);
	return xBalance * y / res;
}

double CRichModel::ProportionOnRightEdgeByImage(int edgeIndex, double x, double y, double proportion) const
{
	double part1 = Edge(edgeIndex).length * y;
	double part2 = proportion * Edge(edgeIndex).length * Edge(edgeIndex).yOfPlanarCoordOfOppositeVert;
	double part3 = Edge(edgeIndex).yOfPlanarCoordOfOppositeVert * x - Edge(edgeIndex).xOfPlanarCoordOfOppositeVert * y;	
	return (part3 + proportion * part1 - part2) / (part3 + part1 - part2);
}

void CRichModel::GetPointByRotatingAround(int edgeIndex, double leftLen, double rightLen, double &xNew, double &yNew) const
{
	xNew = ((leftLen * leftLen - rightLen * rightLen) / Edge(edgeIndex).length + Edge(edgeIndex).length) / 2.0;
	yNew = -sqrt(std::max(leftLen * leftLen - xNew * xNew, 0.0));	
}

void CRichModel::GetPointByRotatingAroundLeftChildEdge(int edgeIndex, double x, double y, double &xNew, double &yNew) const
{
	//double leftLen = sqrt(x * x + y * y);
	//double detaX = x - Edge(edgeIndex).xOfPlanarCoordOfOppositeVert;
	//double detaY = y - Edge(edgeIndex).yOfPlanarCoordOfOppositeVert;
	//double rightLen = sqrt(detaX * detaX + detaY * detaY);
	//GetPointByRotatingAround(Edge(edgeIndex).indexOfLeftEdge, leftLen, rightLen, xNew, yNew);

	//The followings are also okay:

	double leftX = 0;
	double leftY = 0;
	int reverseEdge = Edge(Edge(edgeIndex).indexOfLeftEdge).indexOfReverseEdge;
	double rightX = Edge(reverseEdge).length - Edge(reverseEdge).xOfPlanarCoordOfOppositeVert;
	double rightY = -Edge(reverseEdge).yOfPlanarCoordOfOppositeVert;

	double detaX = rightX - leftX;
	double detaY = rightY - leftY;
	double scale = abs(detaX) + abs(detaY);
	detaX /= scale;
	detaY /= scale;
	double len = sqrt(detaX * detaX + detaY * detaY);
	double unitX = detaX / len;
	double unitY = detaY / len;
	// |unitX   -unitY|
	// |unitY    unitX|
	xNew = unitX * x - unitY * y + leftX;
	yNew = unitY * x + unitX * y + leftY;
}

void CRichModel::GetPointByRotatingAroundRightChildEdge(int edgeIndex, double x, double y, double &xNew, double &yNew) const
{
	//double detaX = x - Edge(edgeIndex).xOfPlanarCoordOfOppositeVert;
	//double detaY = y - Edge(edgeIndex).yOfPlanarCoordOfOppositeVert;
	//double leftLen = sqrt(detaX * detaX + detaY * detaY);
	//detaX = x - Edge(edgeIndex).length;
	//double rightLen = sqrt(detaX * detaX + y * y);
	//GetPointByRotatingAround(Edge(edgeIndex).indexOfRightEdge, leftLen, rightLen, xNew, yNew);

	//The followings are also okay:

	int reverseEdge = Edge(Edge(edgeIndex).indexOfRightEdge).indexOfReverseEdge;
	double rightX = Edge(reverseEdge).length;
	double rightY = 0;
	double leftX = Edge(reverseEdge).length - Edge(reverseEdge).xOfPlanarCoordOfOppositeVert;
	double leftY = -Edge(reverseEdge).yOfPlanarCoordOfOppositeVert;

	double detaX = rightX - leftX;
	double detaY = rightY - leftY;
	double scale = abs(detaX) + abs(detaY);
	detaX /= scale;
	detaY /= scale;
	double len = sqrt(detaX * detaX + detaY * detaY);
	double unitX = detaX / len;
	double unitY = detaY / len;
	// |unitX   -unitY|
	// |unitY    unitX|
	xNew = unitX * x - unitY * y + leftX;
	yNew = unitY * x + unitX * y + leftY;
}


bool CRichModel::HasBeenProcessed() const
{
	return fBePreprocessed;
}

int CRichModel::GetSubindexToVert(int root, int neigh) const
{
	for (int i = 0; i < (int)Neigh(root).size(); ++i)
	{
		if (Edge(Neigh(root)[i].first).indexOfRightVert == neigh)
			return i;
	}
	return -1;
}

CPoint3D CRichModel::ComputeShiftPoint(int indexOfVert) const
{
	return Vert(indexOfVert) + Normal(indexOfVert) * RateOfNormalShift / m_scale;
}

CPoint3D CRichModel::ComputeShiftPoint(int indexOfVert, double factor) const
{
	return Vert(indexOfVert) +  Normal(indexOfVert) * RateOfNormalShift / m_scale * factor;
}

//CPoint3D CRichModel::ComputeShiftPoint(const CPoint3D& pt, int faceId,double factor = 1.0) const
//{
//	
//	return pt +  Normal(Face(faceId)[0]) * RateOfNormalShift / m_scale * factor;
//}

CPoint3D CRichModel::ComputeShiftPoint(const CPoint3D& pt, int simpleEdgeId,double factor = 1.0) const
{
	CPoint3D& normal = Normal(m_SimpleEdges[simpleEdgeId].v1)+Normal(m_SimpleEdges[simpleEdgeId].v2);
	return pt +  normal * RateOfNormalShift / m_scale * factor;
}

CPoint3D CRichModel::ComputeShiftPoint(const CPoint3D& pt, const CFace& face,double factor = 1.0) const
{
	CPoint3D normal = VectorCross(Vert(face[0]),Vert(face[1]),Vert(face[2]));
	normal.Normalize();
	return pt +  normal * RateOfNormalShift / m_scale * factor;
}

//CPoint3D CRichModel::ComputeShiftPoint(const CPoint3D& pt, const int& face_id , double factor = 1.0) const
//{
//    const CFace& face = Face(face_id);
//	CPoint3D normal = VectorCross(Vert(face[0]),Vert(face[1]),Vert(face[2]));
//	normal.Normalize();
//	return pt +  normal * RateOfNormalShift / m_scale * factor;
//}



inline double CRichModel::AngleSum(int vertIndex) const
{
  return m_angle_sum[vertIndex];
	//double angleSum(0);
	//for (int j = 0; j < (int)m_NeighsAndAngles[vertIndex].size(); ++j)
	//{		
	//	angleSum += m_NeighsAndAngles[vertIndex][j].second;			
	//}
	//return angleSum;
}



int CRichModel::GetFirstEdgeIndex(int faceIndex) const
{
	int root = m_Faces[faceIndex][0];
	int subIndex = GetSubindexToVert(root, m_Faces[faceIndex][1]);
	return Neigh(root)[subIndex].first;
}
int CRichModel::GetSecondEdgeIndex(int faceIndex) const
{
	int root = m_Faces[faceIndex][1];
	int subIndex = GetSubindexToVert(root, m_Faces[faceIndex][2]);
	return Neigh(root)[subIndex].first;
}
int CRichModel::GetThirdEdgeIndex(int faceIndex) const
{
	int root = m_Faces[faceIndex][2];
	int subIndex = GetSubindexToVert(root, m_Faces[faceIndex][0]);
	return Neigh(root)[subIndex].first;
}
int CRichModel::GetEdgeIndexFromFace(int faceIndex, int subIndex) const
{
	if (subIndex == 0)
	{
		int edgeIndex = GetFirstEdgeIndex(faceIndex);
		assert (Edge(edgeIndex).indexOfFrontFace == faceIndex);
		return edgeIndex;
	}
	if (subIndex == 1)
	{
		int edgeIndex = GetSecondEdgeIndex(faceIndex);
		assert (Edge(edgeIndex).indexOfFrontFace == faceIndex);
		return edgeIndex;
	}
	if (subIndex == 2)
	{
		int edgeIndex = GetThirdEdgeIndex(faceIndex);
		assert (Edge(edgeIndex).indexOfFrontFace == faceIndex);
		return edgeIndex;
	}
	assert(false);
	return -1;
}

inline const CRichModel::CEdge& CRichModel::GetEdgeFromFace(int faceIndex, int subIndex) const
{
	if (subIndex == 0)
	{
		int edgeIndex = GetFirstEdgeIndex(faceIndex);
		assert (Edge(edgeIndex).indexOfFrontFace == faceIndex);
		return Edge(edgeIndex);
	}
	if (subIndex == 1)
	{
		int edgeIndex = GetSecondEdgeIndex(faceIndex);
		assert (Edge(edgeIndex).indexOfFrontFace == faceIndex);
		return Edge(edgeIndex);
	}
	if (subIndex == 2)
	{
		int edgeIndex = GetThirdEdgeIndex(faceIndex);
		assert (Edge(edgeIndex).indexOfFrontFace == faceIndex);
		return Edge(edgeIndex);
	}
	assert(false);
	return Edge(0);
}

int CRichModel::GetNeighborFaceIndexFromEdge(int edge_index)const
{
	//neighFacesId[j] = initial_model_->Edge(initial_model_->Edge(edgeId[j]).indexOfReverseEdge).indexOfFrontFace;
	return Edge(Edge(edge_index).indexOfReverseEdge).indexOfFrontFace;
}
int CRichModel::GetNeighborFaceIndexFromFace(int face_index,int sub_index)const
{
	int edge_index = GetEdgeIndexFromFace(face_index,sub_index);
	//neighFacesId[j] = initial_model_->Edge(initial_model_->Edge(edgeId[j]).indexOfReverseEdge).indexOfFrontFace;
	return Edge(Edge(edge_index).indexOfReverseEdge).indexOfFrontFace;
}

int CRichModel::GetEdgeIndexFromTwoVertices(int leftVert, int rightVert) const
{
	int subIndex = GetSubindexToVert(leftVert, rightVert);
	assert(subIndex != -1);
	return Neigh(leftVert)[subIndex].first;
}

int CRichModel::GetFaceIndexFromTreeVertices(int v0, int v1, int v2) const//addd by dgg
{
	int edge_id = GetEdgeIndexFromTwoVertices(v0, v1);
	int f0 = Edge(edge_id).indexOfFrontFace;
	int f1 = Edge(Edge(edge_id).indexOfReverseEdge).indexOfFrontFace;
	if (Face(f0)[0] == v2 || Face(f0)[1] == v2 || Face(f0)[2] == v2) return f0;
	if (Face(f1)[0] == v2 || Face(f1)[1] == v2 || Face(f1)[2] == v2) return f1;
	assert(false);
	return 0;
}


double CRichModel::Curvature(int vertIndex) const
{	
	double meanCurvature(0);
	for (int i = 0; i < (int)Neigh(vertIndex).size(); ++i)
	{
		if (!IsExtremeEdge(Neigh(vertIndex)[i].first) && !IsStartEdge(Neigh(vertIndex)[i].first))
		{
			CPoint3D detaVec = Vert(Edge(Neigh(vertIndex)[i].first).indexOfRightVert) - Vert(vertIndex);
			meanCurvature += abs(detaVec ^ Normal(vertIndex)) / (detaVec ^ detaVec) * detaVec.Len();
		}
	}
	meanCurvature /= Neigh(vertIndex).size();
	return meanCurvature;
}

double CRichModel::GetMaxEdgeLength() const
{
	return maxEdgeLength;
}

double CRichModel::GetMeanEdgeLength() const
{
	return meanEdgeLength;
}

double CRichModel::GetEdgeVariance() const
{
	double ave_edge_variance = 0;
	for (int i = 0; i < GetNumOfSimpleEdges(); ++i) {
		auto& simple_e = SimpleEdge(i);
		double e_len = (Vert(simple_e.v1) - Vert(simple_e.v2)).Len();
		ave_edge_variance += (e_len - GetMeanEdgeLength()) * (e_len - GetMeanEdgeLength());
	}
	ave_edge_variance = sqrt(ave_edge_variance / (GetNumOfSimpleEdges() - 1));
	return ave_edge_variance;
}

#endif // !defined(AFX_RICHOBJMODEL_H__EB74D2F8_BA58_480E_9050_6FBC1C83D98B__INCLUDED_)

