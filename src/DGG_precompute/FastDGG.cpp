// FastDGG.cpp: implementation of the CFastDGG class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FastDGG.h"

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFastDGG::CFastDGG(const CRichModel& inputModel, const vector<int> &indexOfSourceVerts) : CPreviousCH(inputModel, indexOfSourceVerts)
{
	nameOfAlgorithm = "FastDGG";
}

CFastDGG::CFastDGG(const CRichModel& inputModel, const vector<int> &indexOfSourceVerts, const set<int>& destinations) : CPreviousCH(inputModel, indexOfSourceVerts), destinations(destinations)
{
	nameOfAlgorithm = "FastDGG";
}

CFastDGG::~CFastDGG()
{
}

void CFastDGG::ExecuteLocally_FastDGG(double eps_vg, set<int> &fixedDests)
{
	if (fComputationCompleted)
		return;
	if (!fLocked)
	{
		fLocked = true;
		nCountOfWindows = 0;
		nMaxLenOfWindowQueue = 0;
		depthOfResultingTree = 0;
		InitContainers();
		nTotalMilliSeconds = GetTickCount();
		BuildSequenceTree(eps_vg, fixedDests);
		nTotalMilliSeconds = GetTickCount() - nTotalMilliSeconds;
		//FillExperimentalResults();
		ClearContainers();

		fComputationCompleted = true;
		fLocked = false;
	}
}

void CFastDGG::InitContainers()
{
	//m_InfoAtAngles.resize(model.GetNumOfEdges());
}

void CFastDGG::ClearContainers()
{
	while (!m_QueueForWindows.empty())
	{
		delete m_QueueForWindows.top().pWindow;
		m_QueueForWindows.pop();
	}

	while (!m_QueueForPseudoSources.empty())
	{
		m_QueueForPseudoSources.pop();
	}
}

void CFastDGG::BuildSequenceTree(double eps_vg, set<int> &fixedDests)
{
	fixedDests.clear();
	epsilon = eps_vg;
	fixedDests.insert(indexOfSourceVerts.begin(), indexOfSourceVerts.end());
	numDirectWindow = 0;
	ComputeChildrenOfSource();
	bool fFromQueueOfPseudoSources = UpdateTreeDepthBackWithChoice();
	double d_max_current = DBL_MAX;
	while (!m_QueueForPseudoSources.empty() || !m_QueueForWindows.empty())
	{
		// DGG Construction Code
		if (numDirectWindow <= 0) break;
		if ((int)m_QueueForWindows.size() > nMaxLenOfWindowQueue)
			nMaxLenOfWindowQueue = (int)m_QueueForWindows.size();
		if (m_QueueForPseudoSources.size() > nMaxLenOfPseudoSources)
			nMaxLenOfPseudoSources = m_QueueForPseudoSources.size();
		double minCurrentDis = DBL_MAX;

		if (fFromQueueOfPseudoSources) //pseudosource
		{
			int indexOfVert = m_QueueForPseudoSources.top().indexOfVert;
			m_QueueForPseudoSources.pop();
			fixedDests.insert(indexOfVert);
		}
		else
		{
			QuoteWindow quoteW = m_QueueForWindows.top();
			m_QueueForWindows.pop();
			if (quoteW.pWindow->maxDistance > GetMinDisOfWindow(*(quoteW.pWindow)))
				ComputeChildrenOfWindow(quoteW);
			delete quoteW.pWindow;
		}

		fFromQueueOfPseudoSources = UpdateTreeDepthBackWithChoice();
	}
}

void CFastDGG::ComputeAndStoreSecondMaxDistanceBound(Window& w, const Window& prevW)
{
}

void CFastDGG::ComputeAndStoreFirstMaxDistanceBound(Window& w, const Window& prevW)
{
	//w.maxDistance = prevW.maxDistance;
	double sourceXCoord = w.xUponUnfolding;
	double sourceYCoord = w.yUponUnfolding;
	double edgelength = model.Edge(w.indexOfCurEdge).length;
	double windowLeftXCoord = w.proportions[0] * edgelength;
	double windowRightXCoord = w.proportions[1] * edgelength;
	double RightXDifference = windowRightXCoord - sourceXCoord; // difference between point b1 to source in term of x coordinate
	double LeftXDifference = windowLeftXCoord - sourceXCoord; // difference between point b0 to source in term of x coordinate
	double LeftDistanceSquared = (LeftXDifference)*(LeftXDifference)+sourceYCoord * sourceYCoord; // (distance of source to b0)^2.
	double RightDistanceSquared = (RightXDifference)*(RightXDifference)+sourceYCoord * sourceYCoord; // (distance of source to b1)^2.
	double isoscelesBottomSquared;

	double RightDistance;
	if (w.distanceRightCHVertex == w.rightLen)RightDistance = w.rightLen;
	else RightDistance = sqrtf(RightDistanceSquared);
	double LeftDistance;
	if (w.distanceLeftCHVertex == w.leftLen) LeftDistance = w.leftLen;
	else LeftDistance = sqrtf(LeftDistanceSquared);

	double triangledistance;
	if (RightDistanceSquared < LeftDistanceSquared) { // extend right hand side of the window since it's the one that's shorter
		float ratio = (LeftDistance / RightDistance);
		double isoscelesRightProjectionX = sourceXCoord + ratio * RightXDifference;
		double isoscelesRightProjectionY = sourceYCoord - ratio * (sourceYCoord);
		isoscelesBottomSquared = (isoscelesRightProjectionX - windowLeftXCoord)*(isoscelesRightProjectionX - windowLeftXCoord)
			+ isoscelesRightProjectionY * isoscelesRightProjectionY; // (isosceles triangle base length)^2
		//isoscelesAltitude = sqrtf(LeftDistanceSquared - 0.25*isoscelesBottomSquared); // calculate triangle altitude
		triangledistance = LeftDistance;
	}
	else { // extend left hand side of the window since it's the one that's shorter
		float ratio = RightDistance / LeftDistance;
		double isoscelesLeftProjectionX = sourceXCoord + ratio * LeftXDifference;
		double isoscelesLeftProjectionY = sourceYCoord - ratio * (sourceYCoord);
		isoscelesBottomSquared = (isoscelesLeftProjectionX - windowRightXCoord)*(isoscelesLeftProjectionX - windowRightXCoord)
			+ isoscelesLeftProjectionY * isoscelesLeftProjectionY; // (isosceles triangle base length)^2
		//isoscelesAltitude = sqrtf(RightDistanceSquared - 0.25*isoscelesBottomSquared); // calculate triangle altitude
		triangledistance = RightDistance;
	}
	//double bound = isoscelesBottomSquared / (8 * epsilon*isoscelesAltitude) + isoscelesAltitude;
	//if (bound < w.maxDistance) w.maxDistance = bound;
	if (isoscelesBottomSquared >= 2 * triangledistance*triangledistance)return; // check if angle > 90 degrees
	double CHdistance_ratio, largerdistance;
	if (w.distanceLeftCHVertex < w.distanceRightCHVertex) {
		CHdistance_ratio = w.distanceLeftCHVertex / w.distanceRightCHVertex;
		largerdistance = w.distanceRightCHVertex;
	}
	else {
		CHdistance_ratio = w.distanceRightCHVertex / w.distanceLeftCHVertex;
		largerdistance = w.distanceLeftCHVertex;
	}
	double scale_factor = 2.25;
	double beta, bottom, oneminratio;
	oneminratio = 1 / (1 - CHdistance_ratio);
	bottom = (largerdistance / triangledistance)*sqrtf(isoscelesBottomSquared);
	if (0.99 < CHdistance_ratio && CHdistance_ratio < 1.01) { beta = 0.5; }
	else {
		beta = oneminratio - sqrtf(oneminratio*(oneminratio - 1) - largerdistance * (largerdistance - bottom) * 2 * scale_factor * epsilon / (bottom*bottom));
	}
	if (beta < 0.5 || beta >= 1 || isnan(beta)) beta = 1;
	double bound = (beta)*(beta)*CHdistance_ratio*bottom*bottom / (2 * scale_factor * epsilon*(largerdistance - bottom)) + CHdistance_ratio * largerdistance;
	if (bound < w.maxDistance && largerdistance > bottom)w.maxDistance = bound;
}

void CFastDGG::ComputeChildrenOfWindow(QuoteWindow& quoteParentWindow)
{
	const Window& w = *quoteParentWindow.pWindow;
	const CRichModel::CEdge& edge = model.Edge(w.indexOfCurEdge);
	double entryProp = model.ProportionOnEdgeByImage(w.indexOfCurEdge, w.xUponUnfolding, w.yUponUnfolding);
	double disToAngle = model.DistanceToIncidentAngle(w.indexOfCurEdge, w.xUponUnfolding, w.yUponUnfolding);
	if (entryProp >= w.proportions[1])
	{
		ComputeTheOnlyLeftChild(w, disToAngle);
		return;
	}
	if (entryProp <= w.proportions[0])
	{
		ComputeTheOnlyRightChild(w, disToAngle);
		return;
	}
	int incidentVertex = edge.indexOfOppositeVert;
	bool fLeftChildToCompute(false), fRightChildToCompute(false);
	bool fWIsWinning(false);
	double totalDis = w.disToRoot + disToAngle;

	if (m_InfoAtAngles[w.indexOfCurEdge].birthTime == -1)
	{
		fLeftChildToCompute = fRightChildToCompute = true;
		fWIsWinning = true;
	}
	else
	{
		if (totalDis < m_InfoAtAngles[w.indexOfCurEdge].disUptodate
			- LENGTH_EPSILON_CONTROL)
		{
			fLeftChildToCompute = fRightChildToCompute = true;
			fWIsWinning = true;
		}
		else
		{
			fLeftChildToCompute = entryProp < m_InfoAtAngles[w.indexOfCurEdge].entryProp;
			fRightChildToCompute = !fLeftChildToCompute;
			fWIsWinning = false;
		}

	}
	if (!fWIsWinning)
	{
		if (fLeftChildToCompute)
		{
			ComputeTheOnlyLeftTrimmedChild(w, disToAngle);
		}
		if (fRightChildToCompute)
		{
			ComputeTheOnlyRightTrimmedChild(w, disToAngle);
		}
		return;
	}

	m_InfoAtAngles[w.indexOfCurEdge].disUptodate = totalDis;
	m_InfoAtAngles[w.indexOfCurEdge].entryProp = entryProp;
	++m_InfoAtAngles[w.indexOfCurEdge].birthTime;

	ComputeLeftTrimmedChildWithParent(w, disToAngle);
	ComputeRightTrimmedChildWithParent(w, disToAngle);
	if (totalDis < m_InfoAtVertices[incidentVertex].disUptodate - LENGTH_EPSILON_CONTROL)
	{
		m_InfoAtVertices[incidentVertex].fParentIsPseudoSource = false;
		++m_InfoAtVertices[incidentVertex].birthTime;
		m_InfoAtVertices[incidentVertex].indexOfParent = w.indexOfCurEdge;
		m_InfoAtVertices[incidentVertex].indexOfRootVertOfParent = w.indexOfRoot;
		m_InfoAtVertices[incidentVertex].level = w.level + 1;
		m_InfoAtVertices[incidentVertex].disUptodate = totalDis;
		m_InfoAtVertices[incidentVertex].entryProp = entryProp;


		double edgelength = model.Edge(w.indexOfCurEdge).length;
		double leftCHVertexX, leftCHVertexY, rightCHVertexX, rightCHVertexY;
		double windowLeftXCoord = model.Edge(w.indexOfCurEdge).length*w.proportions[0];
		double LeftXDifference = windowLeftXCoord - w.xUponUnfolding;
		double windowRightXCoord = model.Edge(w.indexOfCurEdge).length*w.proportions[1];
		double RightXDifference = windowRightXCoord - w.xUponUnfolding;
		double distanceLeft = sqrtf(w.yUponUnfolding*w.yUponUnfolding + LeftXDifference * LeftXDifference);
		double distanceRight = sqrtf(w.yUponUnfolding*w.yUponUnfolding + RightXDifference * RightXDifference);


		leftCHVertexY = (1 - (w.distanceLeftCHVertex / distanceLeft))*w.yUponUnfolding;
		leftCHVertexX = (w.distanceLeftCHVertex / distanceLeft)*(LeftXDifference)+w.xUponUnfolding;


		rightCHVertexY = (1 - (w.distanceRightCHVertex / distanceRight))*w.yUponUnfolding;
		rightCHVertexX = (w.distanceRightCHVertex / distanceRight)*(RightXDifference)+w.xUponUnfolding;


		double disfromLeftCHVertex = model.DistanceToIncidentAngle(w.indexOfCurEdge, leftCHVertexX, leftCHVertexY);
		double disfromRightCHVertex = model.DistanceToIncidentAngle(w.indexOfCurEdge, rightCHVertexX, rightCHVertexY);
		if ((w.distanceLeftCHVertex + disfromLeftCHVertex) > (1 + epsilon)*totalDis && (w.distanceRightCHVertex + disfromRightCHVertex) > (1 + epsilon)*totalDis) {
			AddIntoQueueOfPseudoSources(QuoteInfoAtVertex(m_InfoAtVertices[incidentVertex].birthTime, incidentVertex, totalDis));
		}

	}

}

void CFastDGG::ComputeChildrenOfPseudoSource(int indexOfParentVertex)
{
	if (m_InfoAtVertices[indexOfParentVertex].fParentIsPseudoSource)
		ComputeChildrenOfPseudoSourceFromPseudoSource(indexOfParentVertex);
	else
		ComputeChildrenOfPseudoSourceFromWindow(indexOfParentVertex);
}

void CFastDGG::CreateIntervalChildOfPseudoSource(int source, int subIndexOfIncidentEdge)
{
	int indexOfIncidentEdge = model.Neigh(source)[subIndexOfIncidentEdge].first;
	if (model.IsExtremeEdge(indexOfIncidentEdge))
		return;
	const CRichModel::CEdge& edge = model.Edge(indexOfIncidentEdge);
	int edgeIndex = edge.indexOfRightEdge;
	if (model.IsExtremeEdge(edgeIndex))
		return;
	QuoteWindow quoteW;
	quoteW.pWindow = new Window;
	//quoteW.pWindow->fIsOnLeftSubtree;
	quoteW.pWindow->fParentIsPseudoSource = true;
	//quoteW.pWindow->fDirectParentEdgeOnLeft;
	quoteW.pWindow->fDirectParenIsPseudoSource = true;
	quoteW.pWindow->birthTimeOfParent = m_InfoAtVertices[source].birthTime;
	quoteW.pWindow->indexOfParent = source;
	quoteW.pWindow->indexOfRoot = source;
	quoteW.pWindow->indexOfCurEdge = edgeIndex;
	quoteW.pWindow->level = m_InfoAtVertices[source].level + 1;
	quoteW.pWindow->disToRoot = m_InfoAtVertices[source].disUptodate;
	quoteW.pWindow->proportions[0] = 0;
	quoteW.pWindow->proportions[1] = 1;
	//quoteW.pWindow->entryPropOfParent;
	double rightLen = edge.length;

	quoteW.pWindow->rightLen = rightLen;
	quoteW.pWindow->distanceRightCHVertex = rightLen;

	indexOfIncidentEdge = edge.indexOfLeftEdge;
	double leftLen = model.Edge(indexOfIncidentEdge).length;

	quoteW.pWindow->leftLen = leftLen;
	quoteW.pWindow->distanceLeftCHVertex = leftLen;

	double bottom = model.Edge(edgeIndex).length;
	double x = ((leftLen * leftLen - rightLen * rightLen) / bottom + bottom) / 2.0;
	quoteW.pWindow->xUponUnfolding = x;
	quoteW.pWindow->yUponUnfolding = -sqrt(max(0.0, leftLen * leftLen - x * x));
	AddIntoQueueOfWindows(quoteW);
}

void CFastDGG::FillVertChildOfPseudoSource(int source, int subIndexOfVert)
{
	const CRichModel::CEdge& edge = model.Edge(model.Neigh(source)[subIndexOfVert].first);
	int index = edge.indexOfRightVert;
	double dis = m_InfoAtVertices[source].disUptodate + edge.length;
	if (dis >= m_InfoAtVertices[index].disUptodate - LENGTH_EPSILON_CONTROL)
		return;
	m_InfoAtVertices[index].fParentIsPseudoSource = true;
	++m_InfoAtVertices[index].birthTime;
	m_InfoAtVertices[index].indexOfParent = source;
	//m_InfoAtVertices[index].indexOfRootVertOfParent;
	m_InfoAtVertices[index].level = m_InfoAtVertices[source].level + 1;
	m_InfoAtVertices[index].disUptodate = dis;
	//m_InfoAtVertices[index].entryProp;		
	//if (!model.IsConvexVert(index))
	AddIntoQueueOfPseudoSources(QuoteInfoAtVertex(m_InfoAtVertices[index].birthTime,
		index, dis));
}

void CFastDGG::ComputeTheOnlyLeftChild(const Window& w, double disToAngle)
{
	if (model.IsExtremeEdge(model.Edge(w.indexOfCurEdge).indexOfLeftEdge))
		return;
	QuoteWindow quoteW;
	quoteW.pWindow = new Window;
	quoteW.pWindow->proportions[0] = model.ProportionOnLeftEdgeByImage(w.indexOfCurEdge,
		w.xUponUnfolding, w.yUponUnfolding, w.proportions[0]);
	quoteW.pWindow->proportions[0] = max(0., quoteW.pWindow->proportions[0]);
	quoteW.pWindow->proportions[1] = model.ProportionOnLeftEdgeByImage(w.indexOfCurEdge,
		w.xUponUnfolding, w.yUponUnfolding, w.proportions[1]);
	quoteW.pWindow->proportions[1] = min(1., quoteW.pWindow->proportions[1]);
	if (IsTooNarrowWindow(*quoteW.pWindow))
	{
		delete quoteW.pWindow;
		return;
	}
	quoteW.pWindow->fParentIsPseudoSource = w.fParentIsPseudoSource;
	quoteW.pWindow->fDirectParenIsPseudoSource = false;
	quoteW.pWindow->fDirectParentEdgeOnLeft = true;
	quoteW.pWindow->indexOfCurEdge = model.Edge(w.indexOfCurEdge).indexOfLeftEdge;
	quoteW.pWindow->disToRoot = w.disToRoot;
	quoteW.pWindow->leftLen = w.leftLen;
	quoteW.pWindow->rightLen = disToAngle;
	model.GetPointByRotatingAround(quoteW.pWindow->indexOfCurEdge,
		quoteW.pWindow->leftLen, quoteW.pWindow->rightLen,
		quoteW.pWindow->xUponUnfolding, quoteW.pWindow->yUponUnfolding);
	if (!CheckValidityOfWindow(*quoteW.pWindow))
	{
		delete quoteW.pWindow;
		return;
	}
	quoteW.pWindow->fIsOnLeftSubtree = w.fIsOnLeftSubtree;
	quoteW.pWindow->level = w.level + 1;
	quoteW.pWindow->entryPropOfParent = w.entryPropOfParent;
	quoteW.pWindow->birthTimeOfParent = w.birthTimeOfParent;
	quoteW.pWindow->indexOfParent = w.indexOfParent;
	quoteW.pWindow->indexOfRoot = w.indexOfRoot;

	quoteW.pWindow->maxDistance = w.maxDistance;
	quoteW.pWindow->distanceLeftCHVertex = w.distanceLeftCHVertex;
	quoteW.pWindow->distanceRightCHVertex = w.distanceRightCHVertex;

	AddIntoQueueOfWindows(quoteW);
}

void CFastDGG::ComputeTheOnlyRightChild(const Window& w, double disToAngle)
{
	if (model.IsExtremeEdge(model.Edge(w.indexOfCurEdge).indexOfRightEdge))
		return;
	QuoteWindow quoteW;
	quoteW.pWindow = new Window;
	quoteW.pWindow->proportions[0] = model.ProportionOnRightEdgeByImage(w.indexOfCurEdge,
		w.xUponUnfolding, w.yUponUnfolding, w.proportions[0]);
	quoteW.pWindow->proportions[0] = max(0., quoteW.pWindow->proportions[0]);
	quoteW.pWindow->proportions[1] = model.ProportionOnRightEdgeByImage(w.indexOfCurEdge,
		w.xUponUnfolding, w.yUponUnfolding, w.proportions[1]);
	quoteW.pWindow->proportions[1] = min(1., quoteW.pWindow->proportions[1]);
	if (IsTooNarrowWindow(*quoteW.pWindow))
	{
		delete quoteW.pWindow;
		return;
	}
	quoteW.pWindow->fParentIsPseudoSource = w.fParentIsPseudoSource;
	quoteW.pWindow->fDirectParenIsPseudoSource = false;
	quoteW.pWindow->fDirectParentEdgeOnLeft = false;
	quoteW.pWindow->indexOfCurEdge = model.Edge(w.indexOfCurEdge).indexOfRightEdge;
	quoteW.pWindow->disToRoot = w.disToRoot;
	quoteW.pWindow->leftLen = disToAngle;
	quoteW.pWindow->rightLen = w.rightLen;
	model.GetPointByRotatingAround(quoteW.pWindow->indexOfCurEdge,
		quoteW.pWindow->leftLen, quoteW.pWindow->rightLen,
		quoteW.pWindow->xUponUnfolding, quoteW.pWindow->yUponUnfolding);
	if (!CheckValidityOfWindow(*quoteW.pWindow))
	{
		delete quoteW.pWindow;
		return;
	}
	quoteW.pWindow->level = w.level + 1;
	quoteW.pWindow->birthTimeOfParent = w.birthTimeOfParent;
	quoteW.pWindow->indexOfParent = w.indexOfParent;
	quoteW.pWindow->indexOfRoot = w.indexOfRoot;
	quoteW.pWindow->fIsOnLeftSubtree = w.fIsOnLeftSubtree;
	quoteW.pWindow->entryPropOfParent = w.entryPropOfParent;
	quoteW.pWindow->maxDistance = w.maxDistance;

	quoteW.pWindow->distanceLeftCHVertex = w.distanceLeftCHVertex;
	quoteW.pWindow->distanceRightCHVertex = w.distanceRightCHVertex;

	AddIntoQueueOfWindows(quoteW);
}

void CFastDGG::ComputeTheOnlyLeftTrimmedChild(const Window& w, double disToAngle)
{
	if (model.IsExtremeEdge(model.Edge(w.indexOfCurEdge).indexOfLeftEdge))
		return;
	QuoteWindow quoteW;
	quoteW.pWindow = new Window;
	quoteW.pWindow->proportions[0] = model.ProportionOnLeftEdgeByImage(w.indexOfCurEdge,
		w.xUponUnfolding, w.yUponUnfolding, w.proportions[0]);
	quoteW.pWindow->proportions[0] = max(0., quoteW.pWindow->proportions[0]);
	quoteW.pWindow->proportions[1] = 1;
	if (IsTooNarrowWindow(*quoteW.pWindow))
	{
		delete quoteW.pWindow;
		return;
	}
	quoteW.pWindow->fParentIsPseudoSource = w.fParentIsPseudoSource;
	quoteW.pWindow->fDirectParenIsPseudoSource = false;
	quoteW.pWindow->fDirectParentEdgeOnLeft = true;
	quoteW.pWindow->indexOfCurEdge = model.Edge(w.indexOfCurEdge).indexOfLeftEdge;
	quoteW.pWindow->disToRoot = w.disToRoot;
	quoteW.pWindow->leftLen = w.leftLen;

	quoteW.pWindow->distanceRightCHVertex = quoteW.pWindow->rightLen = disToAngle;
	quoteW.pWindow->distanceLeftCHVertex = w.distanceLeftCHVertex;

	model.GetPointByRotatingAround(quoteW.pWindow->indexOfCurEdge,
		quoteW.pWindow->leftLen, quoteW.pWindow->rightLen,
		quoteW.pWindow->xUponUnfolding, quoteW.pWindow->yUponUnfolding);
	if (!CheckValidityOfWindow(*quoteW.pWindow))
	{
		delete quoteW.pWindow;
		return;
	}
	quoteW.pWindow->level = w.level + 1;
	quoteW.pWindow->birthTimeOfParent = w.birthTimeOfParent;
	quoteW.pWindow->indexOfParent = w.indexOfParent;
	quoteW.pWindow->indexOfRoot = w.indexOfRoot;
	quoteW.pWindow->fIsOnLeftSubtree = w.fIsOnLeftSubtree;
	quoteW.pWindow->entryPropOfParent = w.entryPropOfParent;
	quoteW.pWindow->maxDistance = w.maxDistance;
	ComputeAndStoreFirstMaxDistanceBound(*(quoteW.pWindow), w);
	ComputeAndStoreSecondMaxDistanceBound(*(quoteW.pWindow), w);
	AddIntoQueueOfWindows(quoteW);
}

void CFastDGG::ComputeTheOnlyRightTrimmedChild(const Window& w, double disToAngle)
{
	if (model.IsExtremeEdge(model.Edge(w.indexOfCurEdge).indexOfRightEdge))
		return;
	QuoteWindow quoteW;
	quoteW.pWindow = new Window;
	quoteW.pWindow->proportions[0] = 0;
	quoteW.pWindow->proportions[1] = model.ProportionOnRightEdgeByImage(w.indexOfCurEdge,
		w.xUponUnfolding, w.yUponUnfolding, w.proportions[1]);
	quoteW.pWindow->proportions[1] = min(1., quoteW.pWindow->proportions[1]);
	if (IsTooNarrowWindow(*quoteW.pWindow))
	{
		delete quoteW.pWindow;
		return;
	}
	quoteW.pWindow->fParentIsPseudoSource = w.fParentIsPseudoSource;
	quoteW.pWindow->fDirectParenIsPseudoSource = false;
	quoteW.pWindow->fDirectParentEdgeOnLeft = false;
	quoteW.pWindow->indexOfCurEdge = model.Edge(w.indexOfCurEdge).indexOfRightEdge;
	quoteW.pWindow->disToRoot = w.disToRoot;
	quoteW.pWindow->distanceLeftCHVertex = quoteW.pWindow->leftLen = disToAngle;

	quoteW.pWindow->rightLen = w.rightLen;
	quoteW.pWindow->distanceRightCHVertex = w.distanceRightCHVertex;

	model.GetPointByRotatingAround(quoteW.pWindow->indexOfCurEdge,
		quoteW.pWindow->leftLen, quoteW.pWindow->rightLen,
		quoteW.pWindow->xUponUnfolding, quoteW.pWindow->yUponUnfolding);
	if (!CheckValidityOfWindow(*quoteW.pWindow))
	{
		delete quoteW.pWindow;
		return;
	}
	quoteW.pWindow->level = w.level + 1;
	quoteW.pWindow->birthTimeOfParent = w.birthTimeOfParent;
	quoteW.pWindow->indexOfParent = w.indexOfParent;
	quoteW.pWindow->indexOfRoot = w.indexOfRoot;
	quoteW.pWindow->fIsOnLeftSubtree = w.fIsOnLeftSubtree;
	quoteW.pWindow->entryPropOfParent = w.entryPropOfParent;
	quoteW.pWindow->maxDistance = w.maxDistance;
	ComputeAndStoreFirstMaxDistanceBound(*(quoteW.pWindow), w);
	ComputeAndStoreSecondMaxDistanceBound(*(quoteW.pWindow), w);
	AddIntoQueueOfWindows(quoteW);
}

void CFastDGG::ComputeLeftTrimmedChildWithParent(const Window& w, double disToAngle)
{
	if (model.IsExtremeEdge(model.Edge(w.indexOfCurEdge).indexOfLeftEdge))
		return;
	QuoteWindow quoteW;
	quoteW.pWindow = new Window;
	quoteW.pWindow->proportions[0] = model.ProportionOnLeftEdgeByImage(w.indexOfCurEdge,
		w.xUponUnfolding, w.yUponUnfolding, w.proportions[0]);
	quoteW.pWindow->proportions[0] = max(0., quoteW.pWindow->proportions[0]);
	quoteW.pWindow->proportions[1] = 1;
	if (IsTooNarrowWindow(*quoteW.pWindow))
	{
		delete quoteW.pWindow;
		return;
	}
	quoteW.pWindow->fParentIsPseudoSource = false;
	quoteW.pWindow->fDirectParenIsPseudoSource = false;
	quoteW.pWindow->fDirectParentEdgeOnLeft = true;
	quoteW.pWindow->indexOfCurEdge = model.Edge(w.indexOfCurEdge).indexOfLeftEdge;
	quoteW.pWindow->disToRoot = w.disToRoot;
	quoteW.pWindow->leftLen = w.leftLen;
	quoteW.pWindow->distanceRightCHVertex = quoteW.pWindow->rightLen = disToAngle;
	quoteW.pWindow->distanceLeftCHVertex = w.distanceLeftCHVertex;

	model.GetPointByRotatingAround(quoteW.pWindow->indexOfCurEdge,
		quoteW.pWindow->leftLen, quoteW.pWindow->rightLen,
		quoteW.pWindow->xUponUnfolding, quoteW.pWindow->yUponUnfolding);
	if (!CheckValidityOfWindow(*quoteW.pWindow))
	{
		delete quoteW.pWindow;
		return;
	}
	quoteW.pWindow->level = w.level + 1;
	quoteW.pWindow->birthTimeOfParent = m_InfoAtAngles[w.indexOfCurEdge].birthTime;
	quoteW.pWindow->indexOfParent = w.indexOfCurEdge;
	quoteW.pWindow->indexOfRoot = w.indexOfRoot;
	quoteW.pWindow->fIsOnLeftSubtree = true;
	quoteW.pWindow->entryPropOfParent = m_InfoAtAngles[w.indexOfCurEdge].entryProp;

	quoteW.pWindow->maxDistance = w.maxDistance;
	ComputeAndStoreFirstMaxDistanceBound(*(quoteW.pWindow), w);
	ComputeAndStoreSecondMaxDistanceBound(*(quoteW.pWindow), w);
	AddIntoQueueOfWindows(quoteW);
}

void CFastDGG::ComputeRightTrimmedChildWithParent(const Window& w, double disToAngle)
{
	if (model.IsExtremeEdge(model.Edge(w.indexOfCurEdge).indexOfRightEdge))
		return;
	QuoteWindow quoteW;
	quoteW.pWindow = new Window;
	quoteW.pWindow->proportions[0] = 0;
	quoteW.pWindow->proportions[1] = model.ProportionOnRightEdgeByImage(w.indexOfCurEdge,
		w.xUponUnfolding, w.yUponUnfolding, w.proportions[1]);
	quoteW.pWindow->proportions[1] = min(1., quoteW.pWindow->proportions[1]);
	if (IsTooNarrowWindow(*quoteW.pWindow))
	{
		delete quoteW.pWindow;
		return;
	}
	quoteW.pWindow->fParentIsPseudoSource = false;
	quoteW.pWindow->fDirectParenIsPseudoSource = false;
	quoteW.pWindow->fDirectParentEdgeOnLeft = false;
	quoteW.pWindow->indexOfCurEdge = model.Edge(w.indexOfCurEdge).indexOfRightEdge;
	quoteW.pWindow->disToRoot = w.disToRoot;
	quoteW.pWindow->distanceLeftCHVertex = quoteW.pWindow->leftLen = disToAngle;
	quoteW.pWindow->distanceRightCHVertex = w.distanceRightCHVertex;

	quoteW.pWindow->rightLen = w.rightLen;
	model.GetPointByRotatingAround(quoteW.pWindow->indexOfCurEdge,
		quoteW.pWindow->leftLen, quoteW.pWindow->rightLen,
		quoteW.pWindow->xUponUnfolding, quoteW.pWindow->yUponUnfolding);
	if (!CheckValidityOfWindow(*quoteW.pWindow))
	{
		delete quoteW.pWindow;
		return;
	}
	quoteW.pWindow->fIsOnLeftSubtree = false;
	quoteW.pWindow->birthTimeOfParent = m_InfoAtAngles[w.indexOfCurEdge].birthTime;
	quoteW.pWindow->indexOfParent = w.indexOfCurEdge;
	quoteW.pWindow->indexOfRoot = w.indexOfRoot;
	quoteW.pWindow->level = w.level + 1;
	quoteW.pWindow->entryPropOfParent = m_InfoAtAngles[w.indexOfCurEdge].entryProp;

	quoteW.pWindow->maxDistance = w.maxDistance;
	ComputeAndStoreFirstMaxDistanceBound(*(quoteW.pWindow), w);
	ComputeAndStoreSecondMaxDistanceBound(*(quoteW.pWindow), w);
	AddIntoQueueOfWindows(quoteW);
}