// RichObjModel.cpp: implementation of the CRichModel class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RichModel.h"
#include "dgg_time.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CRichModel::CRichModel(const string& filename) : CBaseModel(filename)
{
	fBePreprocessed = false;
	fLocked = false;
}

void CRichModel::CreateEdgesFromVertsAndFaces()
{
	//printf("vert %d face %d" , GetNumOfVerts() , GetNumOfFaces() );
	m_Edges.reserve(2 * (GetNumOfVerts() + GetNumOfFaces() - 2));
	map<pair<int, int>, int> pondOfUndeterminedEdges;
	int szFaces = GetNumOfFaces();
	for (int i = 0; i < szFaces; ++i)
	{		
		int threeIndices[3];
		for (int j = 0; j < 3; ++j)
		{
			int post = (j + 1) % 3;
			int pre = (j + 2) % 3;
			
			int leftVert = Face(i)[pre];
			int rightVert = Face(i)[j];

			map<pair<int, int>, int>::const_iterator it = pondOfUndeterminedEdges.find(make_pair(leftVert, rightVert));
			if (it != pondOfUndeterminedEdges.end())
			{
				int posInEdgeList = it->second;
				if (m_Edges[posInEdgeList].indexOfOppositeVert != -1)
				{
					printf( "Repeated edges!");
					continue;
				}
				threeIndices[j] = posInEdgeList;
				m_Edges[posInEdgeList].indexOfOppositeVert = Face(i)[post];
				m_Edges[posInEdgeList].indexOfFrontFace = i;				
			}
			else
			{
				CEdge edge;
				edge.indexOfLeftVert = leftVert;
				edge.indexOfRightVert = rightVert;
				edge.indexOfFrontFace = i;
				edge.indexOfOppositeVert = Face(i)[post];
				edge.indexOfReverseEdge = (int)m_Edges.size() + 1;
				edge.length = (Vert(leftVert) - Vert(rightVert)).Len();
				m_Edges.push_back(edge);
				pondOfUndeterminedEdges[make_pair(leftVert, rightVert)] = threeIndices[j] = (int)m_Edges.size() - 1;

				edge.indexOfLeftVert = rightVert;
				edge.indexOfRightVert = leftVert;
				edge.indexOfReverseEdge = (int)m_Edges.size() - 1;
				edge.indexOfOppositeVert = -1;
				m_Edges.push_back(edge);
				pondOfUndeterminedEdges[make_pair(rightVert, leftVert)] = (int)m_Edges.size() - 1;
			}
		}
		for (int j = 0; j < 3; ++j)
		{
			m_Edges[threeIndices[j]].indexOfLeftEdge = Edge(threeIndices[(j + 2) % 3]).indexOfReverseEdge;
			m_Edges[threeIndices[j]].indexOfRightEdge = Edge(threeIndices[(j + 1) % 3]).indexOfReverseEdge;
		}
	}
	m_Edges.swap(vector<CEdge>(m_Edges));
}

void CRichModel::CollectAndArrangeNeighs()
{
	m_nIsolatedVerts = 0;
	vector<int> sequenceOfDegrees(GetNumOfVerts(), 0);	
	m_NeighsAndAngles.resize(GetNumOfVerts());
	for (int i = 0; i < (int)m_NeighsAndAngles.size(); ++i)
	{
		m_NeighsAndAngles[i].resize(1, make_pair(-1, 0));
	}
	for (int i = 0; i < (int)GetNumOfEdges(); ++i)
	{
		const CEdge& edge = Edge(i);
		++sequenceOfDegrees[edge.indexOfLeftVert];
		int &indexOfStartEdge = m_NeighsAndAngles[edge.indexOfLeftVert][0].first;
		if (indexOfStartEdge == -1 || !IsStartEdge(indexOfStartEdge))
		{
			indexOfStartEdge = i;
		}
		else if (IsStartEdge(i))
		{
			m_NeighsAndAngles[edge.indexOfLeftVert].push_back(make_pair(i, 0));
		}
	}
	for (int i = 0; i < GetNumOfVerts(); ++i)
	{
		if (m_NeighsAndAngles[i][0].first == -1)
		{
			m_NeighsAndAngles[i].clear();
			m_nIsolatedVerts++;	
			continue;
		}
		vector<int> startEdges;
		for (int j = 0; j < (int)Neigh(i).size(); ++j)
		{
			startEdges.push_back(Neigh(i)[j].first);
		}	
		m_NeighsAndAngles[i].resize(sequenceOfDegrees[i], make_pair(0, 0));
		int num(0);
		for (int j = 0; j < (int)startEdges.size(); ++j)
		{
			int curEdge = startEdges[j];			
			while (1)
			{
				m_NeighsAndAngles[i][num].first = curEdge;
				++num;
				if (num >= sequenceOfDegrees[i])
					break;
				if (IsExtremeEdge(curEdge))
					break;
				curEdge = Edge(curEdge).indexOfLeftEdge;
				if (curEdge == startEdges[j])
				{
					break;
				}
			}
		}
		if (num != sequenceOfDegrees[i])
		{
			printf("vertex id %d\n" , i );
			//throw "Complex vertices";
			printf("COmplex vertices\n");
		}
	}
}

void CRichModel::ComputeAnglesAroundVerts()
{
	m_neighAngleSum.resize(GetNumOfVerts());
	m_angle_sum.resize(GetNumOfVerts());
	m_FlagsForCheckingConvexVerts.resize(GetNumOfVerts());





	for (int i = 0; i < (int)m_NeighsAndAngles.size(); ++i)
	{
		m_NeighsAndAngles[i].resize(Neigh(i).size());
		m_neighAngleSum[i].resize(Neigh(i).size() + 1);
		fill(m_neighAngleSum[i].begin(), m_neighAngleSum[i].end(), 0);
	}
	for (int i = 0; i < (int)m_NeighsAndAngles.size(); ++i)
	{
		double angleSum(0);
		for (int j = 0; j < (int)m_NeighsAndAngles[i].size(); ++j)
		{
			if (IsExtremeEdge(Neigh(i)[j].first))
				m_NeighsAndAngles[i][j].second = 2 * PI + 0.1;
			else
			{
				int next = j + 1;
				if (next >= (int)m_NeighsAndAngles[i].size())
				{
					next = 0;
				}
				double l = Edge(Neigh(i)[j].first).length;
				double r = Edge(Neigh(i)[next].first).length;
				double b = Edge(Edge(Neigh(i)[j].first).indexOfRightEdge).length;
				m_NeighsAndAngles[i][j].second = acos((l * l + r * r - b * b) / (2 * l * r));
			}
			angleSum += m_NeighsAndAngles[i][j].second;
		}
		m_angle_sum[i] = angleSum;
		//m_FlagsForCheckingConvexVerts[i] = (angleSum < 2 * PI - ToleranceOfConvexAngle);
		m_FlagsForCheckingConvexVerts[i] = (angleSum < 2 * PI);
	}

	for (int i = 0; i < (int)m_NeighsAndAngles.size(); ++i) {
		auto& neighs = Neigh(i);
		auto& sum_angle = m_neighAngleSum[i];
		sum_angle[0] = 0;
		for (int j = 1; j <= neighs.size(); ++j) {
			sum_angle[j] = sum_angle[j - 1] + neighs[j - 1].second;
		}
	}
	//auto& neighs = model.Neigh(source_index);
	//vector<double> sum_angle(neighs.size()+1);
	//sum_angle[0] = 0;
	//for (int j = 1; j <= neighs.size(); ++j) {
	//  sum_angle[j] = sum_angle[j-1] + neighs[j-1].second;
	//}


}

void CRichModel::ComputePlanarCoordsOfIncidentVertForEdges()
{
	for (int i = 0; i < GetNumOfEdges(); ++i)
	{
		if (IsExtremeEdge(i))
			continue;
		double bottom = Edge(i).length;
		double leftLen = Edge(Edge(i).indexOfLeftEdge).length;
		double squareOfLeftLen = leftLen * leftLen;
		double rightLen = Edge(Edge(i).indexOfRightEdge).length;
		double x = (squareOfLeftLen - rightLen * rightLen) / bottom + bottom;
		x /= 2.0;
		m_Edges[i].xOfPlanarCoordOfOppositeVert = x;		
		m_Edges[i].yOfPlanarCoordOfOppositeVert = sqrt(max(0.0, squareOfLeftLen - x * x));
	}
}

void CRichModel::Preprocess()
{
	if (fBePreprocessed)
		return;	
	if (!m_fBeLoaded)
	{
		LoadModel();
	}

	if (!fLocked)
	{
		fLocked = true;
		CreateEdgesFromVertsAndFaces();
		CollectAndArrangeNeighs();	
		ComputeNumOfHoles();
		ComputeAnglesAroundVerts();
		ComputePlanarCoordsOfIncidentVertForEdges();

		ComputeMaxEdgeLength();
		ComputeSimpleEdge();
		fBePreprocessed = true;
		fLocked = false; 		
	}
}

double CRichModel::DistanceToIncidentAngle(int edgeIndex, double x, double y) const
{
	double detaX = x - Edge(edgeIndex).xOfPlanarCoordOfOppositeVert;
	double detaY = y - Edge(edgeIndex).yOfPlanarCoordOfOppositeVert;
	return sqrt(detaX * detaX + detaY * detaY);
}

CPoint3D CRichModel::CombinePointAndNormalTo(const CPoint3D& pt, const CPoint3D& normal)
{
	return pt + normal * RateOfNormalShift;
}

CPoint3D CRichModel::CombineTwoNormalsTo(const CPoint3D& pt1, double coef1, const CPoint3D& pt2, double coef2)
{
	return coef1 * pt1 + coef2 * pt2;
}

void CRichModel::ComputeNumOfHoles()
{
	m_nHoles = 0;
	if (IsClosedModel())
	{
		return;
	}
	set<int> extremeEdges;
	for (int i = 0; i < (int)m_Edges.size(); ++i)
	{
		if (m_Edges[i].indexOfOppositeVert != -1)
			continue;
		extremeEdges.insert(i);
	}		

	while (!extremeEdges.empty())
	{
		++m_nHoles;
		int firstEdge = *extremeEdges.begin();
		int edge = firstEdge;
		do
		{			
			extremeEdges.erase(edge);
			int root = Edge(edge).indexOfRightVert;
			int index = GetSubindexToVert(root, Edge(edge).indexOfLeftVert);
			edge  = Neigh(root)[(index - 1 + (int)Neigh(root).size()) % (int)Neigh(root).size()].first;		
		} while (edge != firstEdge && !extremeEdges.empty());
	}
}

double CRichModel::GetGaussCurvature(int vertIndex) const
{
	double angleSum(0);
	for (int i = 0; i < (int)Neigh(vertIndex).size(); ++i)
	{
		angleSum += Neigh(vertIndex)[i].second;
	}
	return 2 * PI - angleSum;
}

void CRichModel::ComputeMaxEdgeLength()
{
	maxEdgeLength = 0;
	for(int i = 0; i < GetNumOfEdges();++i){
		maxEdgeLength = max(maxEdgeLength,Edge(i).length );
	}
    meanEdgeLength = 0;
	for(int i = 0; i < GetNumOfEdges();++i){
		meanEdgeLength += Edge(i).length;
	}
    meanEdgeLength /= GetNumOfEdges();
}

void CRichModel::ComputeSimpleEdge()
{
	//printf( "simple size %d edge size %d\n"  ,m_SimpleEdges.size(),m_Edges.size());
	ElapasedTime tmp;
	for(int i = 0; i < m_Edges.size();++i){
		m_Edges[i].indexOfSimpleEdge = -1;
	}
	for(int i = 0; i < m_Edges.size();++i){
		m_Edges[i].indexOfReverseEdge;
		CSimpleEdge tmpS(m_Edges[i].indexOfLeftVert,m_Edges[i].indexOfRightVert);
		if( m_Edges[m_Edges[i].indexOfReverseEdge].indexOfSimpleEdge == -1 ){
			m_SimpleEdges.push_back(tmpS);
			m_Edges[i].indexOfSimpleEdge = m_SimpleEdges.size()-1;
		}else{
			 m_Edges[i].indexOfSimpleEdge = m_Edges[m_Edges[i].indexOfReverseEdge].indexOfSimpleEdge;
		}
	}
	//if( m_Edges.size() != m_SimpleEdges.size() * 2 ){
	//	printf("generate simple edge error !\n");
	//}
	//tmp.printTime("generate simple edge");

}