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
#include ".\meshobject.h"

#include <gl/GL.h>
#include <gl/GLU.h>
#include "VFMeshRenderer.h"
#include <string>
#include <Wm4IntpThinPlateSpline2.h>
#include "MeshUtils.h"

MeshObject::MeshObject(void)
{
	m_bExpMapValid = true;
	m_bComputeExpMap = true;
	m_fDecalRadius = 0.3f;

	// create checker texture...
	std::vector<unsigned char> vTexBuf;
	Texture::MakeCheckerImage(512, 64, vTexBuf, true);
	m_texture.InitializeBuffer(512, 512, vTexBuf, GL_RGBA);
	m_texture.SetTextureInfo( GL_TEXTURE_2D );
	m_texture.TextureEnvMode() = GL_DECAL;
	m_texture.SetEdgeAlpha( 0 );
}

MeshObject::~MeshObject(void)
{
}


void MeshObject::ReadCRichModel( CRichModel& model )
{
	rms::VFTriangleMesh readMesh;
	std::string err;
	bool bOK = readMesh.ReadOBJ(model,err);
	if ( ! bOK ) {
		_TCHAR buf[1024];
		//sprintf(buf,"Error opening file %s:\n%s\n", pFilename, err.c_str() );
		MessageBox(NULL, (buf), _T("Error opening file!"), MB_OK);
	}

	readMesh.ClipEarTriangles();

	SetMesh(readMesh);

}


void MeshObject::ReadMeshOBJ( const char * pFilename )
{
	rms::VFTriangleMesh readMesh;
	std::string err;
	bool bOK = readMesh.ReadOBJ(pFilename,err);
	if ( ! bOK ) {
		_TCHAR buf[1024];
		_swprintf(buf,_T("Error opening file %s:\n%s\n"), pFilename, err.c_str() );
		MessageBox(NULL, buf, _T("Error!"), MB_OK);
	}

	readMesh.ClipEarTriangles();

	SetMesh(readMesh);
}

void MeshObject::SetMesh( rms::VFTriangleMesh & mesh )
{
	m_mesh.Clear(false);
	m_mesh.Copy(mesh);

	m_bvTree.SetMesh(&m_mesh);
	//m_expmapgen.SetSurface(&m_mesh, &m_bvTree);
	m_bExpMapValid = false;

	//if ( ! m_mesh.HasUVSet(0) )
	//	m_mesh.AppendUVSet();
	//m_mesh.InitializeUVSet(0);

	float fMin, fMax, fAvg;
	m_mesh.GetEdgeLengthStats(fMin, fMax, fAvg);
	m_fBoundaryWidth =  fMax * 1.0f;

	NotifyMeshModified();
}

void MeshObject::NotifyMeshModified()
{
	m_bvTree.SetMesh(&m_mesh);
	//m_expmapgen.SetSurface(&m_mesh, &m_bvTree);
	m_bExpMapValid = false;

	// find new seed point
	Wml::Vector3f vHit;
	rms::IMesh::TriangleID tID;
	if (! m_bvTree.FindNearest( m_vSeedFrame.Origin(), vHit, tID ) )
		return;
	Wml::Vector3f vVerts[3], vNorms[3];
	m_mesh.GetTriangle(tID, vVerts, vNorms);

	float fBary[3];
	rms::BarycentricCoords( vVerts[0], vVerts[1], vVerts[2], vHit, fBary[0], fBary[1], fBary[2] );
	Wml::Vector3f vHitNormal = fBary[0]*vNorms[0] + fBary[1]*vNorms[1] + fBary[2]*vNorms[2];
	vHitNormal.Normalize();

	m_vSeedFrame.Origin() = vHit;
	m_vSeedFrame.AlignZAxis(vHitNormal);	
}

bool MeshObject::FindIntersection( Wml::Ray3f & vRay, Wml::Vector3f & vHit, Wml::Vector3f & vHitNormal )
{
	rms::IMesh::TriangleID tID;
	if ( ! m_bvTree.FindRayIntersection( vRay.Origin, vRay.Direction, vHit, tID ) )
		return false;
	Wml::Vector3f vVerts[3], vNorms[3];
	m_mesh.GetTriangle(tID, vVerts, vNorms);

	float fBary[3];
	rms::BarycentricCoords( vVerts[0], vVerts[1], vVerts[2], vHit, fBary[0], fBary[1], fBary[2] );
	vHitNormal = 
		fBary[0]*vNorms[0] + fBary[1]*vNorms[1] + fBary[2]*vNorms[2];
	vHitNormal.Normalize();

	return true;
}


bool MeshObject::FindIntersection( Wml::Ray3f & vRay, Wml::Vector3f & vHit, Wml::Vector3f & vHitNormal, int& hit_tri_id )
{
	rms::IMesh::TriangleID tID;
	if ( ! m_bvTree.FindRayIntersection( vRay.Origin, vRay.Direction, vHit, tID ) ) {
		return false;
  }
	Wml::Vector3f vVerts[3], vNorms[3];
  
	m_mesh.GetTriangle(tID, vVerts, vNorms);
  hit_tri_id = tID;
	float fBary[3];
	rms::BarycentricCoords( vVerts[0], vVerts[1], vVerts[2], vHit, fBary[0], fBary[1], fBary[2] );
	vHitNormal = 
		fBary[0]*vNorms[0] + fBary[1]*vNorms[1] + fBary[2]*vNorms[2];
	vHitNormal.Normalize();

	return true;
}

bool MeshObject::FindHitFrame( Wml::Ray3f & vRay, rms::Frame3f & vFrame )
{
	Wml::Vector3f vHit, vHitNormal;
	if ( ! FindIntersection(vRay, vHit, vHitNormal ) ) 
		return false;
	vFrame.Origin() = vHit;
	vFrame.AlignZAxis(vHitNormal);	
	return true;
}
bool MeshObject::FindHitFrame( Wml::Ray3f & vRay, rms::Frame3f & vFrame, int& hit_tri_id)
{
	Wml::Vector3f vHit, vHitNormal;
	if ( ! FindIntersection(vRay, vHit, vHitNormal, hit_tri_id) ) 
		return false;
	vFrame.Origin() = vHit;
	vFrame.AlignZAxis(vHitNormal);	
	return true;
}


void MeshObject::MoveExpMap( rms::Frame3f & vFrame )
{
	m_vSeedFrame.Origin() = vFrame.Origin();
	m_vSeedFrame.AlignZAxis( vFrame.Z() );
	m_bExpMapValid = false;
}

void MeshObject::ScaleExpMap( float fScale )
{
	if ( fScale <= 0 )
		return;
	m_fDecalRadius *= fScale;
	if ( m_fDecalRadius < 0.01f )
		m_fDecalRadius = 0.01f;
	m_bExpMapValid = false;
}

void MeshObject::SetDecalRadius( float fRadius )
{
	m_fDecalRadius = fRadius;
	m_bExpMapValid = false;
}


void MeshObject::RotateExpMap( float fRotate )
{
	Wml::Matrix3f matRotate;
	matRotate.FromAxisAngle(m_vSeedFrame.Z(), fRotate);
	m_vSeedFrame.Rotate(matRotate);
	m_bExpMapValid = false;
}


void MeshObject::ValidateExpMap()
{
	if ( ! m_bComputeExpMap )
		return;
	if ( m_bExpMapValid )
		return;

	float fParamRadius = m_fDecalRadius + m_fBoundaryWidth;
	//m_expmapgen.SetSurfaceDistances(m_vSeedFrame.Origin(), 0.0f, 
		//fParamRadius, &m_vSeedFrame);
	//m_expmapgen.CopyVertexUVs(&m_mesh, 0);

	m_bExpMapValid = true;
}

void MeshObject::Render(bool bWireframe)
{
	glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT | GL_LINE_BIT | GL_POINT_BIT | GL_DEPTH_BUFFER_BIT );

	// render base mesh
	glColor3f(0.6f, 1.0f, 0.6f);
	rms::VFMeshRenderer renderer(&m_mesh);
	renderer.SetNormalMode( rms::VFMeshRenderer::VertexNormals );
	renderer.SetDrawNormals(false);

	if ( bWireframe ) {
		glColor3f(0.3f, 0.8f, 0.3f);
		glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT | GL_LINE_BIT);
		glDisable(GL_LIGHTING);

		// render mesh as z-fill
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		renderer.Render();
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

		glLineWidth(3.0f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	renderer.Render();
	if ( bWireframe )
		glPopAttrib();


	// render support region of decal parameterization
	glDepthFunc(GL_LEQUAL);
	renderer.SetTexture2DMode( rms::VFMeshRenderer::VertexTexture2D_Required );
	glColor4f(0.5f, 0.5f, 1.0f, 1.0f);
	renderer.Render();

	// do texture rendering

	// enable transparency and alpha test
	glDepthFunc(GL_LEQUAL);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glAlphaFunc( GL_NOTEQUAL, 0.0f );
	glEnable( GL_ALPHA_TEST );

	// enable texture
	m_texture.Enable();
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// set texture matrix transformation so that decal is in the right spot
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glTranslatef( 0.5f, 0.5f, 0.0f );	
	float fScale = 1.0f / (m_fDecalRadius * (float)sqrt(2.0f));
	glScalef( fScale, fScale, 1.0f );
	glMatrixMode(GL_MODELVIEW);

	renderer.SetTexture2DMode( rms::VFMeshRenderer::VertexTexture2D_Required );
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	renderer.Render();

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	m_texture.Disable();



	// draw seed point frame
	glDisable(GL_LIGHTING);

	glLineWidth(5.0f);
	glBegin(GL_LINES);
	glColor3f(1.0f,0.0f,0.0f);
	glVertex3fv(m_vSeedFrame.Origin());
	glVertex3fv(m_vSeedFrame.Origin() + 0.025f * m_vSeedFrame.X());
	glColor3f(0.0f,1.0f,0.0f);
	glVertex3fv(m_vSeedFrame.Origin());
	glVertex3fv(m_vSeedFrame.Origin() + 0.025f * m_vSeedFrame.Y());
	glColor3f(0.0f,0.0f,1.0f);
	glVertex3fv(m_vSeedFrame.Origin());
	glVertex3fv(m_vSeedFrame.Origin() + 0.025f * m_vSeedFrame.Z());
	glEnd();


	glPopAttrib();
}



void MeshObject::RenderParamMesh()
{
	glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT | GL_LINE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glLineWidth(2.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor3f(0.0f, 0.0f, 0.0f);

	// set transformation so that decal is in the right spot
	glPushMatrix();
	glLoadIdentity();
	//glTranslatef( 0.5f, 0.5f, 0.0f );	
	//float fScale = 1.0f / (m_fDecalRadius * (float)sqrt(2.0f));
	float fParamRadius = m_fDecalRadius + m_fBoundaryWidth;
	float fScale = 1.5f / (fParamRadius * (float)sqrt(2.0f));
	glScalef( fScale, fScale, 1.0f );

	rms::VFMeshRenderer renderer(&m_mesh);
	renderer.Render_UV(false);	

	glColor3f(1.0f, 0.0f, 0.0f);
	glLineWidth(3.0f);
	glBegin(GL_LINE_LOOP);
	float fSize = m_fDecalRadius / (float)sqrt(2.0f);
	glVertex2f(-fSize, -fSize);
	glVertex2f(fSize, -fSize);
	glVertex2f(fSize, fSize);
	glVertex2f(-fSize, fSize);
	glEnd();

	glPopMatrix();

	glPopAttrib();
}

int MeshObject::FindNearestVertex(const CPoint3D& p)
{
    Wml::Vector3f v_point(p.x,p.y,p.z);
    rms::IMesh::VertexID nearest_vertex_id;
    m_bvTree.FindNearestVtx(v_point , nearest_vertex_id);
    return (int)nearest_vertex_id;
}

int MeshObject::FindNearestTriangle(const CPoint3D& p)
{
    Wml::Vector3f v_point(p.x,p.y,p.z);
    Wml::Vector3f vNearest;
    rms::IMesh::TriangleID nNearestTri;
    //rms::IMesh::VertexID nearest_vertex_id;
    //FindNearest( const Wml::Vector3f & vPoint, Wml::Vector3f & vNearest, IMesh::TriangleID & nNearestTri )
    m_bvTree.FindNearest( v_point, vNearest, nNearestTri );
    return (int)nNearestTri;
}


int MeshObject::FindNearestTriangle(const CPoint3D& p, CPoint3D& vNearest)
{
    Wml::Vector3f v_point(p.x,p.y,p.z);
    Wml::Vector3f vNearest_wml;
    rms::IMesh::TriangleID nNearestTri;
    //rms::IMesh::VertexID nearest_vertex_id;
    //FindNearest( const Wml::Vector3f & vPoint, Wml::Vector3f & vNearest, IMesh::TriangleID & nNearestTri )
    m_bvTree.FindNearest( v_point, vNearest_wml, nNearestTri );
    vNearest= CPoint3D(vNearest_wml.X(),vNearest_wml.Y(),vNearest_wml.Z());
    return (int)nNearestTri;
}


