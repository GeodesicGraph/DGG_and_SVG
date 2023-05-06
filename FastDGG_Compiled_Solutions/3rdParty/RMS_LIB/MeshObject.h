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
#include "Texture.h"
#include "Frame.h"
#include "ICH/Point3D.h"
#include "ICH/RichModel.h"

class MeshObject
{
public:
	MeshObject(void);
	~MeshObject(void);

	void ReadMeshOBJ( const char * pFilename );
	void ReadCRichModel( CRichModel& model );
	void SetMesh( rms::VFTriangleMesh & mesh );
	void NotifyMeshModified();

	void Render(bool bWireframe = false);
	void RenderParamMesh();

	rms::VFTriangleMesh & GetMesh() { return m_mesh; }
	rms::IMeshBVTree & GetBVTree() { return m_bvTree; }

	bool FindIntersection( Wml::Ray3f & vRay, Wml::Vector3f & vHit, Wml::Vector3f & vHitNormal );
  bool FindIntersection( Wml::Ray3f & vRay, Wml::Vector3f & vHit, Wml::Vector3f & vHitNormal, int& hit_tri_id );

	bool FindHitFrame( Wml::Ray3f & vRay, rms::Frame3f & vFrame );
	bool FindHitFrame( Wml::Ray3f & vRay, rms::Frame3f & vFrame , int& hit_tri_id );
    int FindNearestVertex(const CPoint3D& p);
    int FindNearestTriangle(const CPoint3D& p);
    int MeshObject::FindNearestTriangle(const CPoint3D& p, CPoint3D& vNearest);

	rms::Frame3f & ExpMapFrame() { return m_vSeedFrame; }
	void MoveExpMap( rms::Frame3f & vFrame );
	void ScaleExpMap( float fScale );
	void RotateExpMap( float fRotate );
	void ValidateExpMap();
	void SetDecalRadius( float fRadius );

	//rms::ExpMapGenerator * GetExpMapGen() { return & m_expmapgen; }

protected:
	rms::VFTriangleMesh m_mesh;
	rms::IMeshBVTree m_bvTree;

	rms::Frame3f m_vSeedFrame;
	float m_fDecalRadius;
	float m_fBoundaryWidth;

	//rms::ExpMapGenerator m_expmapgen;
	bool m_bExpMapValid;
	bool m_bComputeExpMap;

	Texture m_texture;
};
