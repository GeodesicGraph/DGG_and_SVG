// Author: Ryan Schmidt  (rms@unknownroad.com)  http://www.unknownroad.com
// Copyright (c) 2008. All Rights Reserved.
// The ExpMapDemo package is supplied "AS IS". The Author disclaims all warranties, expressed or implied, including, 
// without limitation, the warranties of merchantability and of fitness for any purpose. The Author assume no liability for 
// direct, indirect, incidental, special, exemplary, or consequential damages, which may result from the use of the 
// ExpMapDemo package, even if advised of the possibility of such damage. Permission is hereby granted to use, copy, 
// modify, and distribute this source code, or portions hereof, for any purpose, without fee, subject to the following restrictions:
// 1) The origin of this source code must not be misrepresented.
// 2) This Copyright notice may not be removed or altered from any source or altered source distribution.
// The Author specifically permits, without fee, the use of this source code as a component in commercial products.
#include "StdAfx.h"
#include ".\meshutils.h"

#include "VectorUtil.h"

#include <GL/gl.h>
#include <GL/glu.h>

using namespace rms;

MeshUtils::MeshUtils(void)
{
}

MeshUtils::~MeshUtils(void)
{
}



bool MeshUtils::MergeVertices( rms::VFTriangleMesh & mesh, float fThreshold )
{
	std::set<IMesh::VertexID> vUsed;
	std::vector<IMesh::EdgeID> vMerge;

	fThreshold *= fThreshold;

	// find edges that are too short
	VFTriangleMesh::edge_iterator cure(mesh.BeginEdges()), ende(mesh.EndEdges());
	while ( cure != ende ) {
		IMesh::EdgeID eID = *cure++;
		if ( mesh.IsBoundaryEdge(eID) )
			continue;

		Wml::Vector3f vEdge[2];
		mesh.GetEdge(eID, vEdge);
		float fLength = (vEdge[0]-vEdge[1]).SquaredLength();
		if ( fLength < fThreshold ) {
			IMesh::VertexID v[2];   IMesh::TriangleID t[2];
			mesh.GetEdge(eID, v, t);
			if ( mesh.IsBoundaryVertex(v[0]) || mesh.IsBoundaryVertex(v[1]) )
				continue;

			IMesh::VertexID nTri[6];
			mesh.GetTriangle(t[0],&nTri[0]);
			mesh.GetTriangle(t[1],&nTri[3]);
			bool bOK = true;
			for ( int k = 0; k < 6; ++k ) {
				if ( vUsed.find(nTri[k]) != vUsed.end() ) 
					bOK = false;
			}
			if ( bOK ) {
				vMerge.push_back(eID);
				for ( int k = 0; k < 6; ++k )
					vUsed.insert(nTri[k]);
			}
		}
	}


	// collapse edges
	size_t nCollapse = vMerge.size();
	int nReallyCollapsed = 0;
	for ( unsigned int i = 0; i < nCollapse; ++i ) {
		if ( mesh.CollapseEdge( vMerge[i] ) ) {
			++nReallyCollapsed;
		}
	}
	//_RMSInfo("Collapsed %d edges (of %d possibilities)\n", nReallyCollapsed, nCollapse);

	return (nReallyCollapsed > 0);
}




bool MeshUtils::CollapseTipFaces( rms::VFTriangleMesh & mesh, bool bAll )
{
	bool bCollapsedAny = false;

	std::set<IMesh::EdgeID> vStartBoundaryEdges;
	VFTriangleMesh::edge_iterator cure(mesh.BeginEdges()), ende(mesh.EndEdges());
	while ( cure != ende ) {
		IMesh::EdgeID eID = *cure++;
		if ( mesh.IsBoundaryEdge(eID) )
			vStartBoundaryEdges.insert(eID);
	}

	bool bCollapsed = true;
	while ( bCollapsed ) {
		bCollapsed = false;

		std::vector<IMesh::VertexID> vTips;
		std::vector<IMesh::EdgeID> vEdges;

		VFTriangleMesh::vertex_iterator curv(mesh.BeginVertices()), endv(mesh.EndVertices());
		while ( curv != endv ) {
			IMesh::VertexID vID = *curv++;

			vEdges.resize(0);
			
			IMesh::VtxNbrItr itr;
			itr.vID = vID;
			mesh.BeginVtxEdges(itr);
			IMesh::EdgeID eID = mesh.GetNextVtxEdges(itr);
			while ( eID != IMesh::InvalidID ) {
				vEdges.push_back(eID);
				eID = mesh.GetNextVtxEdges(itr);
			}

			size_t nEdges = vEdges.size();
			if ( nEdges != 3 )
				continue;
			//if ( mesh.IsBoundaryEdge(vEdges[0]) || mesh.IsBoundaryEdge(vEdges[1]) || mesh.IsBoundaryEdge(vEdges[2]) )
			//	continue;
			if ( vStartBoundaryEdges.find(vEdges[0]) != vStartBoundaryEdges.end() ||
				 vStartBoundaryEdges.find(vEdges[1]) != vStartBoundaryEdges.end() ||
				 vStartBoundaryEdges.find(vEdges[2]) != vStartBoundaryEdges.end() )
				 continue;

			vTips.push_back(vID);
		}

		size_t nTips = vTips.size();
		int nClipped = 0;
		for ( unsigned int i = 0; i < nTips; ++i ) {
			IMesh::VertexID vID = vTips[i];
			if ( ! mesh.IsVertex(vID) )
				continue;

			std::set<IMesh::VertexID> vNbrSet;
			std::vector<IMesh::VertexID> vNbrs;
			std::vector<IMesh::TriangleID> vTris;

			IMesh::VertexID nVerts[3];
			IMesh::VtxNbrItr itr;
			itr.vID = vID;
			mesh.BeginVtxTriangles(itr);
			IMesh::TriangleID tID = mesh.GetNextVtxTriangle(itr);
			while ( tID != IMesh::InvalidID ) {
				mesh.GetTriangle(tID, nVerts);
				for ( int k = 0; k < 3; ++k ) {
					if ( nVerts[k] == vID )
						continue;
					if ( vNbrSet.insert(nVerts[k]).second == false )
						vNbrs.push_back(nVerts[k]);
				}

				vTris.push_back(tID);
				tID = mesh.GetNextVtxTriangle(itr);
			}
					
			// sanity check
			if ( vNbrs.size() != 3 || vTris.size() != 3 ) {
				//DebugBreak();
				continue;
			}

			mesh.RemoveVertex( vID );

			// check if we should add in face (would create invalid mesh if 
			// we have a tetrahedra
			std::vector<IMesh::VertexID> nNbrVerts(3);
			std::sort( vNbrs.begin(), vNbrs.end() );
			IMesh::VtxNbrItr itr2(vNbrs[0]);
			mesh.BeginVtxTriangles(itr2);
			tID = mesh.GetNextVtxTriangle(itr2);
			bool bSkipTri = false;
			while ( tID != IMesh::InvalidID && ! bSkipTri) {
				mesh.GetTriangle(tID, &nNbrVerts[0]);
				std::sort(nNbrVerts.begin(), nNbrVerts.end());
				if ( vNbrs[0] == nNbrVerts[0] && vNbrs[1] == nNbrVerts[1] && vNbrs[2] == nNbrVerts[2] )
					bSkipTri = true;
				tID = mesh.GetNextVtxTriangle(itr2);
			}
			if ( ! bSkipTri ) {

				// hacky bit to maintain vertex ordering
				IMesh::VertexID vFirst, vSecond, vThird;
				for ( int j = 0; j < 3; ++j ) {
					if ( nVerts[j] == vID ) {
						vFirst = nVerts[(j+1)%3];  vSecond = nVerts[(j+2)%3];
					}
				}
				for ( int j = 0; j < 3; ++j ) {
					if ( vNbrs[j] != vFirst && vNbrs[j] != vSecond )
						vThird = vNbrs[j];
				}
				mesh.AppendTriangle( vFirst, vSecond, vThird );

			} else {

				// might have created a fin - get rid of it
				for ( int k = 0; k < 3; ++k ) {
					if ( mesh.GetEdgeCount(vNbrs[k]) == 2 )
						mesh.RemoveVertex(vNbrs[k]);
				}

				// TODO: this removal might in turn create more fins - really ought to keep going...

			}

			++nClipped;
			bCollapsedAny = true;
		}

		//_RMSInfo("Clipped %d tips (of %d possible)\n", nClipped, nTips);
		bCollapsed = nClipped > 0 && bAll;
	}

	return bCollapsedAny;
}




bool MeshUtils::CollapseFinFaces( rms::VFTriangleMesh & mesh, bool bAll )
{
	bool bCollapsedAny = false;

	std::set<IMesh::EdgeID> vStartBoundaryEdges;
	VFTriangleMesh::edge_iterator cure(mesh.BeginEdges()), ende(mesh.EndEdges());
	while ( cure != ende ) {
		IMesh::EdgeID eID = *cure++;
		if ( mesh.IsBoundaryEdge(eID) )
			vStartBoundaryEdges.insert(eID);
	}

	bool bCollapsed = true;
	while ( bCollapsed ) {
		bCollapsed = false;

		std::vector<IMesh::VertexID> vFins;
		std::vector<IMesh::EdgeID> vEdges;

		VFTriangleMesh::vertex_iterator curv(mesh.BeginVertices()), endv(mesh.EndVertices());
		while ( curv != endv ) {
			IMesh::VertexID vID = *curv++;

			vEdges.resize(0);
			
			IMesh::VtxNbrItr itr;
			itr.vID = vID;
			mesh.BeginVtxEdges(itr);
			IMesh::EdgeID eID = mesh.GetNextVtxEdges(itr);
			while ( eID != IMesh::InvalidID ) {
				vEdges.push_back(eID);
				eID = mesh.GetNextVtxEdges(itr);
			}

			size_t nEdges = vEdges.size();
			if ( nEdges != 2 )
				continue;
//			if ( mesh.IsBoundaryEdge(vEdges[0]) || mesh.IsBoundaryEdge(vEdges[1]) )
			if ( vStartBoundaryEdges.find(vEdges[0]) != vStartBoundaryEdges.end() ||
				 vStartBoundaryEdges.find(vEdges[1]) != vStartBoundaryEdges.end() )
				continue;

			vFins.push_back(vID);
		}

		size_t nFins = vFins.size();
		int nClipped = 0;
		for ( unsigned int i = 0; i < nFins; ++i ) {
			IMesh::VertexID vID = vFins[i];
			mesh.RemoveVertex(vID);
			++nClipped;
			bCollapsedAny = true;
		}

		//_RMSInfo("Clipped %d tips (of %d possible)\n", nClipped, nTips);
		bCollapsed = nClipped > 0 && bAll;
	}

	return bCollapsedAny;
}







bool MeshUtils::CheckForFins( VFTriangleMesh & mesh )
{
	int nFins = 0;
	VFTriangleMesh::vertex_iterator curv(mesh.BeginVertices()), endv(mesh.EndVertices());
	while ( curv != endv ) {
		IMesh::VertexID vID = *curv++;

		if ( ! mesh.IsBoundaryVertex(vID) && mesh.GetEdgeCount(vID) == 2 )
			++nFins;
	}
    if ( nFins > 0 ){
        fprintf( stderr , "Found %d fins!\n" , nFins);
        assert(false);

        //_RMSInfo("Found %d fins!\n", nFins);
    }
	return (nFins > 0);
}

bool MeshUtils::CheckMeshValidity( VFTriangleMesh & mesh, bool bTopologyChecks )
{
	bool bOK = true;

	// check verts
	VFTriangleMesh::vertex_iterator curv(mesh.BeginVertices()), endv(mesh.EndVertices());
	while ( curv != endv ) {
		IMesh::VertexID vID = *curv++;
		if ( ! mesh.IsVertex(vID) ) {
			bOK = false; DebugBreak();
		}

		// do nbr iters
		VFTriangleMesh::VtxNbrItr triItr(vID);
		mesh.BeginVtxTriangles(triItr);
		IMesh::TriangleID tID = mesh.GetNextVtxTriangle(triItr);
		while ( tID != IMesh::InvalidID ) {
			if ( ! mesh.IsTriangle(tID) ) {
				bOK = false; DebugBreak();
			}
			tID = mesh.GetNextVtxTriangle(triItr);
		}

		VFTriangleMesh::VtxNbrItr edgeItr(vID);
		mesh.BeginVtxEdges(edgeItr);
		IMesh::EdgeID eID = mesh.GetNextVtxEdges(edgeItr);
		while ( eID != IMesh::InvalidID ) {
			if ( ! mesh.IsEdge(eID) ) {
				bOK = false; DebugBreak();
			}
			eID = mesh.GetNextVtxEdges(edgeItr);
		}

		if ( bTopologyChecks ) {
			bool bIsBoundary = mesh.IsBoundaryVertex(vID);

			if ( ! bIsBoundary && mesh.GetTriangleCount(vID) < 3 ) {
				bOK = false; DebugBreak();
			}

			if ( ! bIsBoundary && mesh.GetEdgeCount(vID) < 3 ) {
				bOK = false; DebugBreak();
			}
		}
	}


	// check that each tri vert is a real vert
	VFTriangleMesh::triangle_iterator curt(mesh.BeginTriangles()), endt(mesh.EndTriangles());
	while ( curt != endt ) {
		IMesh::TriangleID tID = *curt++;
		if (! mesh.IsTriangle(tID) ) {
			bOK = false; DebugBreak();
		}
		IMesh::VertexID nTri[3];
		mesh.GetTriangle(tID, nTri);
		for ( int j = 0; j < 3; ++j ) {
			if ( ! mesh.IsVertex( nTri[j]) ) {
				bOK = false; DebugBreak();
			}
		}

		Wml::Vector3f vVerts[3];
		mesh.GetTriangle(tID, vVerts);
		float fArea = Area(vVerts[0], vVerts[1], vVerts[2]);
		if ( fArea < 0.00001f ) {
			bOK = false; DebugBreak();
		}
	}

	// check that each edge vert is a real vert
	VFTriangleMesh::edge_iterator cure(mesh.BeginEdges()), ende(mesh.EndEdges());
	while ( cure != ende ) {
		IMesh::EdgeID eID = *cure++;
		if (! mesh.IsEdge(eID) ) {
			bOK = false; DebugBreak();
		}
		IMesh::VertexID nVtx[2];
		IMesh::TriangleID nTri[2];
		mesh.GetEdge(eID, nVtx, nTri);
		for ( int j = 0; j < 2; ++j ) {
			if ( ! mesh.IsVertex( nVtx[j]) ) {
				bOK = false; DebugBreak();
			}
			if ( nTri[j] != IMesh::InvalidID && ! mesh.IsTriangle( nTri[j]) ) {
				bOK = false; DebugBreak();
			}
		}
	}

	return bOK;
}





void MeshUtils::GetSubMesh( rms::VFTriangleMesh & original, 
							rms::VFTriangleMesh & submesh, 
							const std::set<rms::IMesh::TriangleID> & vTris )
{
	// make list of verts to save
	IMesh::VertexID nTri[3];
	std::set<IMesh::VertexID> vVerts;
	std::set<IMesh::TriangleID>::const_iterator curt(vTris.begin()), endt(vTris.end());
	while ( curt != endt ) {
		IMesh::TriangleID tID = *curt++;
		original.GetTriangle(tID, nTri);
		vVerts.insert(nTri[0]);   vVerts.insert(nTri[1]);  vVerts.insert(nTri[2]);
	}

	// add verts
	std::vector<IMesh::VertexID> vVertMap(original.GetMaxVertexID(), IMesh::InvalidID);
	std::set<IMesh::VertexID>::iterator curv(vVerts.begin()), endv(vVerts.end());
	while ( curv != endv ) {
		IMesh::VertexID vID = *curv++;
		Wml::Vector3f vVertex, vNormal;
		original.GetVertex(vID, vVertex, &vNormal);
		vVertMap[vID] = submesh.AppendVertex(vVertex, &vNormal);
	}

	// add tris
	curt = vTris.begin();
	while ( curt != endt ) {
		IMesh::TriangleID tID = *curt++;
		original.GetTriangle(tID, nTri);
		submesh.AppendTriangle( vVertMap[nTri[0]], vVertMap[nTri[1]], vVertMap[nTri[2]] );
	}
}




void MeshUtils::SetMeshXYZtoUV( VFTriangleMesh & mesh, const Wml::Vector3f & vSetNormal, int nCoordMap[2],
							   std::vector<Wml::Vector3f> * pvXYZPositions, std::vector<Wml::Vector3f> * pvXYZNormals )
{
	if ( pvXYZPositions )
		pvXYZPositions->resize( mesh.GetMaxVertexID() );
	if ( pvXYZNormals )
		pvXYZNormals->resize( mesh.GetMaxVertexID() );

	VFTriangleMesh::vertex_iterator curv(mesh.BeginVertices()), endv(mesh.EndVertices());
	while ( curv != endv ) {
		IMesh::VertexID vID = *curv++;
		Wml::Vector3f vVertex, vNormal;  Wml::Vector2f vUV;
		mesh.GetVertex(vID, vVertex, &vNormal);
		if ( pvXYZPositions )
			(*pvXYZPositions)[vID] = vVertex;
		if ( pvXYZNormals ) 
			(*pvXYZNormals)[vID] = vNormal;
		if ( ! mesh.GetUV(vID, 0, vUV) )
			vUV = Wml::Vector2f(-1, -1);
		vVertex = Wml::Vector3f::ZERO;
		vVertex[ nCoordMap[0] ] = vUV.X();
		vVertex[ nCoordMap[1] ] = vUV.Y();
		mesh.SetVertex(vID, vVertex, & vSetNormal);
	}	
}




void MeshUtils::CopyUVs( VFTriangleMesh & from, VFTriangleMesh & to, IMesh::UVSetID nFromSetID, IMesh::UVSetID nToSetID,
								bool bBoundaryOnly )
{
	if ( ! to.HasUVSet(nToSetID) ) {
		while ( to.AppendUVSet() != nToSetID )
			;
		to.InitializeUVSet( nToSetID );
	}
	to.ClearUVSet( nToSetID );

	Wml::Vector2f vUV;
	VFTriangleMesh::vertex_iterator curv(from.BeginVertices()), endv(from.EndVertices());
	while ( curv != endv ) {
		IMesh::VertexID vID = *curv++;
		if ( bBoundaryOnly && ! from.IsBoundaryVertex(vID) )
			continue;
		from.GetUV(vID, nFromSetID, vUV);
		to.SetUV(vID, nToSetID, vUV);
	}
}


void MeshUtils::CopyUVs( IMesh & from, VFTriangleMesh & to, IMesh::UVSetID nFromSetID, IMesh::UVSetID nToSetID,
								const std::vector<IMesh::VertexID> & vVertexMap, bool bBoundaryOnly )
{
	if ( ! to.HasUVSet(nToSetID) ) {
		while ( to.AppendUVSet() != nToSetID )
			;
		to.InitializeUVSet( nToSetID );
	}
	to.ClearUVSet( nToSetID );

	Wml::Vector2f vUV;
	IMesh::IVtxIterator curv(from.BeginIVertices()), endv(from.EndIVertices());
	while ( curv != endv ) {
		IMesh::VertexID vFromID = *curv; curv++;
		IMesh::VertexID vToID = vVertexMap[vFromID];

		if ( bBoundaryOnly && ! from.IsBoundaryVertex(vToID) )
			continue;
		from.GetUV(vFromID, nFromSetID, vUV);
		to.SetUV(vToID, nToSetID, vUV);
	}
}





void MeshUtils::CopyBoundaryUVs( VFTriangleMesh & from, VFTriangleMesh & to, IMesh::UVSetID nFromSetID, IMesh::UVSetID nToSetID,
		BoundaryLoopMap & bmap )
{
	if ( ! to.HasUVSet(nToSetID) ) {
		while ( to.AppendUVSet() != nToSetID )
			;
		to.InitializeUVSet( nToSetID );
	}
	to.ClearUVSet( nToSetID );

	Wml::Vector2f vUV;
	size_t nCount = bmap.vFrom.size();
	for ( unsigned int i = 0; i < nCount; ++i ) {
		IMesh::VertexID vFromID = bmap.vFrom[i];
		IMesh::VertexID vToID = bmap.vTo[i];
		from.GetUV( vFromID, nFromSetID, vUV );
		to.SetUV( vToID, nToSetID, vUV );
	}
}




void MeshUtils::MergeBoundaryLoops( rms::VFTriangleMesh & mesh,
		const std::vector<IMesh::VertexID> & vFrom, const std::vector<IMesh::VertexID> & vTo  )
{
	size_t nCount = vFrom.size();
	for ( unsigned int i = 0; i < nCount; ++i ) {
		IMesh::VertexID vFromID = vFrom[i];
		IMesh::VertexID vToID = vTo[i];
		
		// make list of tris to rewrite
		std::vector<IMesh::VertexID> vTris;
		IMesh::VtxNbrItr itr(vFromID);
		mesh.BeginVtxTriangles(itr);
		IMesh::TriangleID tID = mesh.GetNextVtxTriangle(itr);
		while ( tID != IMesh::InvalidID ) {
			vTris.push_back(tID);
			tID = mesh.GetNextVtxEdges(itr);
		}

		size_t nTris = vTris.size();
		for ( unsigned int k = 0; k < nTris; ++k ) {
			IMesh::VertexID nTri[3];
			mesh.GetTriangle( vTris[k], nTri );
			for ( int j = 0; j < 3; ++j )
				nTri[j] = (nTri[j] == vFromID) ? vToID : nTri[j];
			if ( ! mesh.SetTriangle( vTris[k], nTri[0], nTri[1], nTri[2] ) )
				DebugBreak();
		}

		// ok save to remove vert
		mesh.RemoveVertex(vFromID);
	}
}


void MeshUtils::MergeAllBoundaryLoops( VFTriangleMesh & mesh, std::vector< std::vector<IMesh::VertexID> > * vToLoops, float fDistThresh )
{
	// have to iterate because IDs change after each merge!!

	bool bDone = false;
	while (! bDone ) {
		std::vector< std::vector< IMesh::VertexID > > vLoops;
		FindBoundaryLoops( mesh, vLoops );

		std::vector< BoundaryLoopMap > vMaps;
		FindBoundaryLoopMaps( mesh, vLoops, mesh, vLoops, vMaps, fDistThresh );

		if ( vMaps.size() == 0 ) {
			bDone = true;
		} else {
			if ( vToLoops != NULL )
				vToLoops->push_back( vMaps[0].vTo );
			MergeBoundaryLoops( mesh, vMaps[0].vFrom, vMaps[0].vTo );
		}
	}

}




void MeshUtils::FindBoundaryLoopMaps(
		VFTriangleMesh & mesh1, const std::vector< std::vector< IMesh::VertexID > > & vLoops1,
		VFTriangleMesh & mesh2, const std::vector< std::vector< IMesh::VertexID > > & vLoops2,
		std::vector< BoundaryLoopMap > & vMaps, float fDistThresh )
{
	fDistThresh *= fDistThresh; 

	bool bSameMesh = ( (&mesh1) == (&mesh2) );

	size_t nCount1 = vLoops1.size();
	size_t nCount2 = vLoops2.size();
	for ( unsigned int n1 = 0; n1 < nCount1; ++n1 ) {
	for ( unsigned int n2 = 0; n2 < nCount2; ++n2 ) {
		if ( bSameMesh && n1 == n2 )
			continue;
		if ( vLoops1[n1].size() != vLoops2[n2].size() ) 
			continue;

		const std::vector< IMesh::VertexID > & vLoopA = vLoops1[n1];
		size_t nCountA = vLoopA.size();
		const std::vector< IMesh::VertexID > & vLoopB = vLoops2[n2];
		size_t nCountB = vLoopB.size();

		// find vertex w/ zero-distance in both loops
		Wml::Vector3f vA, vB; 
		IMesh::VertexID vIDA = vLoopA[0];
		mesh1.GetVertex(vIDA, vA);
		IMesh::VertexID vIDB = IMesh::InvalidID;
		int iLoopB = 0;
		float fMinDist = 9999999.0f;
		for ( unsigned int i = 0; i < nCountB; ++i ) {
			mesh2.GetVertex( vLoopB[i], vB );
			float fLenSqr = (vA-vB).SquaredLength();
			if ( fLenSqr < fMinDist && fLenSqr < fDistThresh ) {
				vIDB = vLoopB[i];
				iLoopB = i;
				fMinDist = fLenSqr;
			}
		}
		if ( vIDB == IMesh::InvalidID )
			continue;
	
		// ok got one - now generate correspondence
		BoundaryLoopMap bmap;
		bmap.nLoop1Index = n1;
		bmap.nLoop2Index = n2;
		bmap.vFrom = vLoopA;

		bmap.vTo.push_back(vIDB);

		// figure out if B is going in same or opposite direction
		bool bFailed = false;
		iLoopB = (iLoopB + 1) % (int)nCountB;
		mesh2.GetVertex( vLoopB[ iLoopB ], vB );
		mesh1.GetVertex( vLoopA[1], vA );
		if ( (vA-vB).SquaredLength() < fDistThresh ) {
			while ( bmap.vTo.size() < nCountA ) {
				bmap.vTo.push_back( vLoopB[iLoopB] );
				iLoopB = (iLoopB + 1) % (int)nCountB;
			}

		} else {
			mesh1.GetVertex( vLoopA.back(), vA );
			if ( (vA-vB).SquaredLength() < fDistThresh ) {
				iLoopB -= 2;
				if ( iLoopB < 0 ) 
					iLoopB = (int)nCountB + iLoopB;
				while ( bmap.vTo.size() < nCountA ) {
					bmap.vTo.push_back( vLoopB[iLoopB] );
					iLoopB--;
					if ( iLoopB < 0 ) 
						iLoopB = (int)nCountB + iLoopB;
				}
			} else
				bFailed = true;
		}

		if ( ! bFailed )
			vMaps.push_back( bmap );
		else{
			//_RMSInfo("Correspondence search failed in MeshUtils::GetBoundaryLoopMaps\n");
            assert(false);
        }
	}
	}
}

void MeshUtils::FindBoundaryLoops( rms::VFTriangleMesh & mesh, std::vector< std::vector< rms::IMesh::TriangleID > > & vLoops )
{
	std::set< IMesh::EdgeID > vBEdges;

	VFTriangleMesh::edge_iterator cure(mesh.BeginEdges()), ende(mesh.EndEdges());
	while ( cure != ende ) {
		IMesh::EdgeID eID = *cure++;
		if ( mesh.IsBoundaryEdge(eID) )
			vBEdges.insert(eID); 
	}

	// ok start loop at first edge
	while ( ! vBEdges.empty() ) {
		IMesh::EdgeID eID = *vBEdges.begin();
		vBEdges.erase(eID);

		IMesh::VertexID nVerts[2];   IMesh::TriangleID nTris[2];
		mesh.GetEdge(eID, nVerts, nTris);
		std::vector<IMesh::VertexID> vLoop;
		vLoop.push_back( nVerts[0] );  
		vLoop.push_back( nVerts[1] );
		IMesh::VertexID nLastVID = vLoop.back();

		while ( nLastVID != vLoop.front() ) {

			VFTriangleMesh::VtxNbrItr itr(nLastVID);
			mesh.BeginVtxEdges(itr);
			eID = mesh.GetNextVtxEdges(itr);
			while (eID != IMesh::InvalidID) {
				if ( vBEdges.find(eID) != vBEdges.end() ) {
					mesh.GetEdge(eID, nVerts, nTris);
					nLastVID = (nVerts[0] == nLastVID) ? nVerts[1] : nVerts[0];
					if ( nLastVID != vLoop.front() )
						vLoop.push_back( nLastVID );
					vBEdges.erase(eID);
					eID = IMesh::InvalidID;		
				} else
					eID = mesh.GetNextVtxEdges(itr);
			}

		}

		vLoops.push_back(vLoop);
	}
}





void MeshUtils::FindOneRing( VFTriangleMesh & mesh, IMesh::VertexID vID, std::vector<IMesh::VertexID> & vOneRing, bool bOrdered )
{
	if ( bOrdered )
		DebugBreak();

	vOneRing.resize(0);

	VFTriangleMesh::VtxNbrItr itr(vID);
	mesh.BeginVtxEdges(itr);
	IMesh::EdgeID eID = mesh.GetNextVtxEdges(itr);
	while ( eID != IMesh::InvalidID ) {
		IMesh::VertexID nVerts[2], nTris[2];
		mesh.GetEdge(eID, nVerts, nTris);
		vOneRing.push_back( (nVerts[0] == vID) ? nVerts[1] : nVerts[0] );
		eID = mesh.GetNextVtxEdges(itr);
	}
}




void MeshUtils::GetBoundaryEdgeStats( VFTriangleMesh & mesh, float & fMin, float & fMax, float & fAverage )
{
	fMax = 0.0f;
	fMin = std::numeric_limits<float>::max();
	double fAverageEdge = 0.0f;
	Wml::Vector3f vVertices[2];

	int nCount = 0;
	VFTriangleMesh::edge_iterator cure( mesh.BeginEdges() ), ende( mesh.EndEdges() );
	while ( cure != ende ) {
		IMesh::EdgeID eID = *cure++;
		if ( ! mesh.IsBoundaryEdge(eID) )
			continue;
		
		mesh.GetEdge(eID, vVertices);
		float fLen = (vVertices[0] - vVertices[1]).Length();
		if ( fLen < fMin )
			fMin = fLen;
		if ( fLen > fMax )
			fMax = fLen;
		fAverageEdge += fLen;
		++nCount;
	}

	fAverage = (float)(fAverageEdge / (double)nCount);	
}


void MeshUtils::GetBoundaryBoundingBox( VFTriangleMesh & mesh, Wml::AxisAlignedBox3f & bounds )
{
	bool bInitialized = false;

	VFTriangleMesh::vertex_iterator curv(mesh.BeginVertices()), endv(mesh.EndVertices());
	while ( curv != endv ) {
		IMesh::VertexID vID = *curv++;
		if ( ! mesh.IsBoundaryVertex(vID) )
			continue;
		Wml::Vector3f v;
		mesh.GetVertex(vID, v);
		if ( ! bInitialized ) {
			bounds = Wml::AxisAlignedBox3f(v.X(),v.X(), v.Y(),v.Y(), v.Z(),v.Z());
			bInitialized = true;
		} else {
			rms::Union(bounds, v);
		}
	}
}


void MeshUtils::DrawBoundaryEdges( rms::VFTriangleMesh & mesh, const Wml::Vector3f & vColor )
{
	glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT);
	glDisable(GL_LIGHTING);
	glColor3f(vColor.X(), vColor.Y(), vColor.Z());
	glLineWidth(5.0f);

	glBegin(GL_LINES);

	VFTriangleMesh::edge_iterator cure(mesh.BeginEdges()), ende(mesh.EndEdges());
	while ( cure != ende ) {
		IMesh::EdgeID eID = *cure++;
		if ( ! mesh.IsBoundaryEdge(eID) )
			continue;

		Wml::Vector3f vVerts[2];
		mesh.GetEdge(eID, vVerts);
		glVertex3fv(vVerts[0]);
		glVertex3fv(vVerts[1]);
	}

	glEnd();
	glPopAttrib();
}




void MeshUtils::DrawFrontEdges( rms::VFTriangleMesh & mesh )
{
	if ( ! mesh.HasScalarSet(0))
		return;

	glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT);
	glDisable(GL_LIGHTING);
	glLineWidth(4.0f);
	glDepthFunc(GL_LEQUAL);

	glColor4f(0.0f,0.0f,1.0f,1.0f);

	glBegin(GL_LINES);

	VFTriangleMesh::edge_iterator cure(mesh.BeginEdges()), ende(mesh.EndEdges());
	while ( cure != ende ) {
		IMesh::EdgeID eID = *cure++;
		IMesh::VertexID nVerts[2], nTris[2];
		mesh.GetEdge(eID, nVerts, nTris);
		IMesh::VertexID nTriVerts[3];
		mesh.GetTriangle(nTris[0],nTriVerts);
		bool bFront1 = true;    float fScalar = 0;
		for ( int k = 0; k < 3; ++k )
			bFront1 = bFront1 && mesh.GetScalar(nTriVerts[k], 0, fScalar) && fScalar < 1.0f;
		mesh.GetTriangle(nTris[1],nTriVerts);
		bool bFront2 = true;
		for ( int k = 0; k < 3; ++k )
			bFront2 = bFront2 && mesh.GetScalar(nTriVerts[k], 0, fScalar) && fScalar < 1.0f;
		bool bFrontEdge = bFront1 ^ bFront2;

		if ( bFrontEdge ) {
			Wml::Vector3f vEdge[2];
			mesh.GetEdge(eID, vEdge);
			glVertex3fv(vEdge[0]);
			glVertex3fv(vEdge[1]);
		}
	}

	glEnd();

	glPopAttrib();
}




// recursive K-ring iteration - repeats itself lots of times, need to be smarter...
//static void InsertKRing( VFTriangleMesh * pMesh, IMesh::VertexID vID, std::set<IMesh::VertexID> & vDone, int K )
//{
//	if ( K == 0 )
//		return;
//
//	VFTriangleMesh::VtxNbrItr itr(vID);
//	pMesh->BeginVtxEdges(itr);
//	IMesh::EdgeID eID = pMesh->GetNextVtxEdges(itr);
//	while ( eID != IMesh::InvalidID ) {
//		IMesh::VertexID nVtx[2], nTri[2];
//		pMesh->GetEdge( eID, nVtx, nTri );
//		IMesh::VertexID vOtherID = (nVtx[0] == vID) ? nVtx[1] : nVtx[0];
//		vDone.insert(vOtherID);
//
//		// not efficient! 
//		InsertKRing(pMesh, vOtherID, vDone, K-1);
//
//		eID = pMesh->GetNextVtxEdges(itr);
//	}
//}


Wml::Vector3f MeshUtils::GetAverageNormal( VFTriangleMesh & mesh, IMesh::VertexID vID, unsigned int nKRings )
{
	Wml::Vector3f vCenter, vVertex, vNormal, vAvgNormal(Wml::Vector3f::ZERO);
	float fWeightSum = 0.0f;

	mesh.GetVertex(vID, vCenter);

	VFTriangleMesh::VtxNbrItr itr(vID);
	mesh.BeginVtxEdges(itr);
	IMesh::EdgeID eID = mesh.GetNextVtxEdges(itr);
	while ( eID != IMesh::InvalidID ) {
		IMesh::VertexID nVtx[2], nTri[2];
		mesh.GetEdge( eID, nVtx, nTri );
		IMesh::VertexID vOtherID = (nVtx[0] == vID) ? nVtx[1] : nVtx[0];

		mesh.GetVertex(vOtherID, vVertex, &vNormal);

		//float fWeight = 1.0f;
		float fWeight = 1.0f / (vVertex - vCenter).Length();

		vAvgNormal += vNormal;
		fWeightSum += fWeight;

		eID = mesh.GetNextVtxEdges(itr);
	}	

	vAvgNormal *= 1.0f / fWeightSum;
	vAvgNormal.Normalize();

	return vAvgNormal;
}


void MeshUtils::SmoothNormals( VFTriangleMesh & mesh, float fAlpha )
{
	std::vector<Wml::Vector3f> vNewNormals( mesh.GetMaxVertexID() );
	VFTriangleMesh::vertex_iterator curv(mesh.BeginVertices()), endv(mesh.EndVertices());
	while ( curv != endv ) {
		IMesh::VertexID vID = *curv++;
		Wml::Vector3f vSmoothNorm = GetAverageNormal(mesh, vID);
		if ( fAlpha < 1 ) {
			Wml::Vector3f vOrigNorm;
			mesh.GetNormal(vID, vOrigNorm);
			vSmoothNorm = fAlpha * vSmoothNorm + (1-fAlpha) * vOrigNorm;
			vSmoothNorm.Normalize();
		}
		vNewNormals[vID] = vSmoothNorm;
	}
	curv = mesh.BeginVertices();
	while ( curv != endv ) {
		IMesh::VertexID vID = *curv++;
		mesh.SetNormal(vID, vNewNormals[vID]);
	}
}



Wml::Vector3f MeshUtils::EstimateNormal( VFTriangleMesh & mesh, IMesh::VertexID vID, NormalEstMode eMode )
{
	float fWeightSum = 0;
	Wml::Vector3f vEstimate(Wml::Vector3f::ZERO), vTri[3], vNormal;

	IMesh::VtxNbrItr itr(vID);
	mesh.BeginVtxTriangles(itr);
	IMesh::TriangleID tID = mesh.GetNextVtxTriangle(itr);
	while ( tID != IMesh::InvalidID ) {

		if ( eMode == AreaWeightedFaceAvg ) {
			mesh.GetTriangle(tID, vTri);
			float fWeight;
			vNormal = Normal(vTri[0], vTri[1], vTri[2], &fWeight );
			vEstimate += fWeight * vNormal;
			fWeightSum += fWeight;
		}

		tID = mesh.GetNextVtxTriangle(itr);
	}

	if ( eMode == AreaWeightedFaceAvg ) {
		vEstimate *= 1.0f / fWeightSum;
	}
	vEstimate.Normalize();
	return vEstimate;
}


void MeshUtils::EstimateNormals( VFTriangleMesh & mesh, NormalEstMode eMode )
{
	VFTriangleMesh::vertex_iterator curv(mesh.BeginVertices()), endv(mesh.EndVertices());
	while ( curv != endv ) {
		IMesh::VertexID vID = *curv++;
		Wml::Vector3f vEstimate = EstimateNormal(mesh, vID, eMode);
		mesh.SetNormal(vID, vEstimate);
	}	
}







void MeshUtils::ScaleMesh( VFTriangleMesh & mesh, const Wml::Vector3f & vScale, const Wml::Vector3f & vCenter )
{
	// TODO: handle normals properly...

	Wml::Vector3f vVertex, vNormal;
	VFTriangleMesh::vertex_iterator curv(mesh.BeginVertices()), endv(mesh.EndVertices());
	while ( curv != endv ) {
		IMesh::VertexID vID = *curv++;

		mesh.GetVertex( vID, vVertex, &vNormal );
		vVertex -= vCenter;
		vVertex.X() *= vScale.X();  vVertex.Y() *= vScale.Y();   vVertex.Z() *= vScale.Z();
		vVertex += vCenter;

		mesh.SetVertex( vID, vVertex, &vNormal );
	}
}


void MeshUtils::TranslateMesh( VFTriangleMesh & mesh, const Wml::Vector3f & vTranslate )
{
	Wml::Vector3f vVertex;
	VFTriangleMesh::vertex_iterator curv(mesh.BeginVertices()), endv(mesh.EndVertices());
	while ( curv != endv ) {
		IMesh::VertexID vID = *curv++;
		mesh.GetVertex( vID, vVertex );
		vVertex += vTranslate;
		mesh.SetVertex( vID, vVertex );
	}
}









