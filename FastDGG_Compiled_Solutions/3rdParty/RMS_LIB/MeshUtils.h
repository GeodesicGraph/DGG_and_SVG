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
#pragma once

#include "VFTriangleMesh.h"
#include "IMeshBVTree.h"

namespace rms {

class MeshUtils
{
public:

	static void GetSubMesh( VFTriangleMesh & original, 
							VFTriangleMesh & submesh, 
							const std::set<IMesh::TriangleID> & vTris );

	//! coord map would be [0,1] to, for example, map UV to XY, or [0,2] to map UV to XZ
	static void SetMeshXYZtoUV( VFTriangleMesh & mesh, const Wml::Vector3f & vNormal, int nCoordMap[2],  
		std::vector<Wml::Vector3f> * pvXYZPositions = NULL, std::vector<Wml::Vector3f> * pvXYZNormals = NULL );

	//! assumes that from & to are same mesh (must have same VIDs). Will create toSetID if necessary.
	static void CopyUVs( VFTriangleMesh & from, VFTriangleMesh & to, IMesh::UVSetID nFromSetID, IMesh::UVSetID nToSetID,
		bool bBoundaryOnly = false);

	//! vVertexMap maps 'from' verts to 'to' verts
	static void CopyUVs( IMesh & from, VFTriangleMesh & to, IMesh::UVSetID nFromSetID, IMesh::UVSetID nToSetID,
		const std::vector<IMesh::VertexID> & vVertexMap, bool bBoundaryOnly = false);

	static bool CollapseTipFaces( VFTriangleMesh & mesh, bool bAll = true );
	static bool CollapseFinFaces( VFTriangleMesh & mesh, bool bAll = true );
	static bool MergeVertices( VFTriangleMesh & mesh, float fThreshold );

	static bool CheckForFins( VFTriangleMesh & mesh );
	static bool CheckMeshValidity( VFTriangleMesh & mesh, bool bTopologyChecks );

	//! bOrdered flag currently not implemented
	static void FindOneRing( VFTriangleMesh & mesh, IMesh::VertexID vID, std::vector<IMesh::VertexID> & vOneRing, bool bOrdered = false );

	static void FindBoundaryLoops( VFTriangleMesh & mesh, std::vector< std::vector< IMesh::VertexID > > & vLoops );

	struct BoundaryLoopMap {
		unsigned int nLoop1Index;
		unsigned int nLoop2Index;
		std::vector<IMesh::VertexID> vFrom;
		std::vector<IMesh::VertexID> vTo;
	};

	//! find corresponding boundary loops in two meshes. If mesh2 = mesh1, will intelligently skip repeats
	static void FindBoundaryLoopMaps(
		VFTriangleMesh & mesh1, const std::vector< std::vector< IMesh::VertexID > > & vLoops1,
		VFTriangleMesh & mesh2, const std::vector< std::vector< IMesh::VertexID > > & vLoops2,
		std::vector< BoundaryLoopMap > & vMaps, float fDistThresh = 0.000001f );

	static void MergeBoundaryLoops( VFTriangleMesh & mesh,
		const std::vector<IMesh::VertexID> & vFrom, const std::vector<IMesh::VertexID> & vTo );

	//! pass non-null vToLoops to have returned the VIDs of loop verts that were kept
	static void MergeAllBoundaryLoops( VFTriangleMesh & mesh, 
		std::vector< std::vector<IMesh::VertexID> > * vToLoops = NULL,
		float fDistThresh = 0.000001f  );

	static void CopyBoundaryUVs( VFTriangleMesh & from, VFTriangleMesh & to, IMesh::UVSetID nFromSetID, IMesh::UVSetID nToSetID,
		BoundaryLoopMap & bmap );

	static void GetBoundaryBoundingBox( VFTriangleMesh & mesh, Wml::AxisAlignedBox3f & bounds );


	static void GetBoundaryEdgeStats( VFTriangleMesh & mesh, float & fMin, float & fMax, float & fAvg );

	// currently only does one-ring...
	static Wml::Vector3f GetAverageNormal( VFTriangleMesh & mesh, IMesh::VertexID vID, unsigned int nKRings = 1 );
	// ! fAlpha in range [0,1], 1 means replace w/ smoothed normal
	static void SmoothNormals( VFTriangleMesh & mesh, float fAlpha = 1.0f ); 

	enum NormalEstMode {
		AreaWeightedFaceAvg
	};
	static Wml::Vector3f EstimateNormal( VFTriangleMesh & mesh, IMesh::VertexID vID, NormalEstMode eMode = AreaWeightedFaceAvg );
	static void EstimateNormals( VFTriangleMesh & mesh, NormalEstMode eMode = AreaWeightedFaceAvg );

	//! note: scale is currently not applied to normals...
	static void ScaleMesh( VFTriangleMesh & mesh, const Wml::Vector3f & vScale, const Wml::Vector3f & vCenter = Wml::Vector3f::ZERO );
	static void TranslateMesh( VFTriangleMesh & mesh, const Wml::Vector3f & vTranslate );


	//! appends faces of vID to vSelection
	static void SelectFaces( VFTriangleMesh & mesh, IMesh::VertexID & vID, std::set<IMesh::TriangleID> & vSelection );

	//! appends all faces connected to vVerts to vSelection
	static void SelectFaces( VFTriangleMesh & mesh, const std::vector<IMesh::VertexID> & vVerts, std::set<IMesh::TriangleID> & vSelection );

	// TODO: this would be more efficient if we could find the boundary verts of a selection...
	//! appends all one-ring neighbours of current selection. pNewVerts returns added verts, if requested
	static void GrowSelection( VFTriangleMesh & mesh, 
							   std::set<IMesh::TriangleID> & vSelection, 
							   std::set<IMesh::VertexID> * pNewVerts );

	static void DrawBoundaryEdges( VFTriangleMesh & mesh, const Wml::Vector3f & vColor = Wml::Vector3f::UNIT_Z );
	static void DrawFrontEdges( VFTriangleMesh & mesh );



private:
	MeshUtils(void);
	~MeshUtils(void);
};


/*
 * This one-ring neighbour iteration constructs a list [parents] of all
 * the neighbouring vertices of [vertid] which are in [knownverts]
 */
class MakeOneRingListCallback : public IMesh::NeighborTriCallback
{
public:
	MakeOneRingListCallback( IMesh::VertexID vertid, VFTriangleMesh * mesh) {
		vID = vertid;
		pMesh = mesh;
	}
	IMesh::VertexID vID;
	std::set<IMesh::VertexID> vOneRing;
	VFTriangleMesh * pMesh;

	virtual void NextTriangle( IMesh::TriangleID tID ) {
		IMesh::VertexID nTri[3];
		pMesh->GetTriangle(tID, nTri);
		for ( int k = 0; k < 3; ++k ) {
			if ( nTri[k] != vID )
				vOneRing.insert( nTri[k] );
		}
	}
};


}  // end namespace rms
