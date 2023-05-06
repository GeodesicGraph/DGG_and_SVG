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
#ifndef __RMS_MESH_BVTREE_H__
#define __RMS_MESH_BVTREE_H__

#include <Wm4AxisAlignedBox3.h>
#include <Wm4Ray3.h>

#include "IMesh.h"
#include "MemoryPool.h"



namespace rms {

class IMeshBVTree
{
public:
	IMeshBVTree( );
	IMeshBVTree( IMesh * pMesh );

	void SetMesh( IMesh * pMesh );

	void Clear();

	void GetMeshBounds( Wml::AxisAlignedBox3f & bounds );

	bool FindRayIntersection( const Wml::Vector3f & vOrigin, const Wml::Vector3f & vDirection,
							  Wml::Vector3f & vHit, IMesh::TriangleID & nHitTri );

	bool FindNearest( const Wml::Vector3f & vPoint, Wml::Vector3f & vNearest, IMesh::TriangleID & nNearestTri );
  bool IMeshBVTree::FindNearest(const Wml::Vector3f & vPoint, IMesh::TriangleID & nNearestTri);

	bool FindNearestVtx( const Wml::Vector3f & vPoint, IMesh::VertexID & nNearestVtx );

	void ExpandAll();

protected:
	IMesh * m_pMesh;


	class IMeshBVNode {
	public:
		unsigned int ID;
		Wml::AxisAlignedBox3f Box;
		IMeshBVTree::IMeshBVNode * pLeft;
		IMeshBVTree::IMeshBVNode * pRight;

		union {
			struct {
				unsigned int Index : 31;
				unsigned int NotLeaf : 1;
			} TriangleList;
			unsigned int ID;
		} Triangle;

		inline bool HasChildren() { 
				return pLeft != NULL && pRight != NULL; }

		inline bool IsLeaf() {
				return Triangle.TriangleList.NotLeaf == 0; }

		inline void SetIndex( unsigned int nIndex ) {
				Triangle.TriangleList.NotLeaf = 1;
				Triangle.TriangleList.Index = nIndex; }
		inline unsigned int GetIndex() { 
				return Triangle.TriangleList.Index; }

		inline void SetTriangleID( unsigned int nID ) {
				Triangle.TriangleList.NotLeaf = 0;
				Triangle.ID = nID; }
		inline unsigned int GetTriangleID() {
				return Triangle.ID; }

		inline void Union( Wml::Vector3f & vPoint );
	};


	
	MemoryPool<IMeshBVNode> m_vNodePool;
	IMeshBVNode * m_pRoot;
	unsigned int m_nNodeIDGen;
	IMeshBVNode * GetNewNode();

	void Initialize();


	struct TriangleEntry {
		unsigned int nJump : 12;		// 0xFFF
		unsigned int nNodeID  : 20;		
		IMesh::TriangleID triID;
	};
	std::vector<TriangleEntry> m_vTriangles;
	unsigned int m_nMaxTriangle;

	inline unsigned int GetNextEntryIdx( IMeshBVNode * pNode, unsigned int nIndex );
	inline void SetJump( unsigned int nIndex, unsigned int nNext );


	struct Ray {
		Ray( const Wml::Vector3f & vOrigin, const Wml::Vector3f & vDirection );
		Wml::Ray3f wmlRay;
		Wml::Vector3f origin;
		Wml::Vector3f direction;
		Wml::Vector3f inv_direction;
		int sign[3];
	};

	void ComputeBox( IMeshBVNode * pNode );
	void ExpandNode( IMeshBVNode * pNode );
	bool TestIntersection( IMeshBVNode * pNode, Ray & ray, float & fNear, float & fFar );
	float MinDistance( IMeshBVNode * pNode, const Wml::Vector3f & vPoint );

	//! recursive intersection test
	bool FindRayIntersection( IMeshBVTree::IMeshBVNode * pNode, Ray & ray, 
							  Wml::Vector3f & vHit, float & fNearest, IMesh::TriangleID & nHitTri );

	bool FindNearest( IMeshBVTree::IMeshBVNode * pNode, const Wml::Vector3f & vPoint, 
					  Wml::Vector3f & vNearest, float & fNearest, IMesh::TriangleID & nNearestTri );

	void ExpandAll( IMeshBVTree::IMeshBVNode * pNode );
};



void IMeshBVTree::IMeshBVNode::Union( Wml::Vector3f & point )
{
	if ( point.X() < Box.XMin() )
		Box.XMin() = point.X();
	if ( point.X() > Box.XMax() )
		Box.XMax() = point.X();
	if ( point.Y() < Box.YMin() )
		Box.YMin() = point.Y();
	if ( point.Y() > Box.YMax() )
		Box.YMax() = point.Y();
	if ( point.Z() < Box.ZMin() )
		Box.ZMin() = point.Z();
	if ( point.Z() > Box.ZMax() )
		Box.ZMax() = point.Z();
}


unsigned int IMeshBVTree::GetNextEntryIdx( IMeshBVTree::IMeshBVNode * pNode, unsigned int nIndex )
{
	nIndex = nIndex + m_vTriangles[nIndex].nJump;
	while ( nIndex < m_nMaxTriangle ) {
		if ( m_vTriangles[nIndex].nNodeID == pNode->ID )
			return nIndex;
		else
			++nIndex;	// linear iteration to next entry...
	}
	return IMesh::InvalidID;
}

void IMeshBVTree::SetJump( unsigned int nIndex, unsigned int nNext )
{
	if ( nNext - nIndex < 0xFFF ) {
		m_vTriangles[nIndex].nJump = (nNext - nIndex) & 0xFFF;
	} else {
		m_vTriangles[nIndex].nJump = 0xFFF;
	}
	if ( m_vTriangles[nIndex].nJump == 0 )
		DebugBreak();
}


}  // namespace rmsmesh

#endif // __RMS_MESH_BVTREE_H__