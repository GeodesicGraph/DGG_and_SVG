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
#include ".\vfmeshrenderer.h"

#include <gl/GL.h>
#include <gl/GLU.h>

using namespace rms;

VFMeshRenderer::VFMeshRenderer( VFTriangleMesh * pMesh )
{
	m_pMesh = pMesh;

	m_eNormalMode = FaceNormals;
	m_eColorMode = NoColors;
	m_eTexture2DMode = NoTexture2D;
	
	m_bDrawNormals = false;
	
	m_bEnableScalarColor = false;

}

VFMeshRenderer::~VFMeshRenderer(void)
{
}

void VFMeshRenderer::Render()
{
	IMesh::VertexID vTri[3];
	Wml::Vector3f vVtx[3], vNorm[3];
	Wml::Vector2f vTriUV[3];
	Wml::Vector2f vInvalidUV[3];
	vInvalidUV[0] = vInvalidUV[1] = vInvalidUV[2] = Wml::Vector2f(99999.0f, 99999.0f);
	float vTriScalar[3];
	bool bTriHasScalars;
	Wml::Vector2f * pCurUV;
	Wml::ColorRGBA cColor =  Wml::ColorRGBA::WHITE;

	VFTriangleMesh::triangle_iterator 
		cur( m_pMesh->BeginTriangles() ),
		end( m_pMesh->EndTriangles() );


	bool bShowScalars = false;
	if ( m_bEnableScalarColor && m_pMesh->HasScalarSet(0) )
		bShowScalars = true;


	glPushAttrib(GL_ENABLE_BIT);
	if ( bShowScalars )
		glDisable(GL_LIGHTING);


	glBegin(GL_TRIANGLES);
	while ( cur != end ) {
		IMesh::TriangleID nTriID = *cur;	cur++;

		m_pMesh->GetTriangle( nTriID, vTri );
		m_pMesh->GetTriangle( nTriID, vVtx, vNorm );

		if ( m_eTexture2DMode == VertexTexture2D || m_eTexture2DMode == VertexTexture2D_Required ) {
			pCurUV = ( m_pMesh->GetTriangleUV( nTriID, 0, vTriUV )) ? vTriUV : vInvalidUV;
		}
		if ( m_eTexture2DMode == VertexTexture2D_Required && pCurUV == vInvalidUV )
			continue;

		if ( bShowScalars ) {
			bTriHasScalars = true;
			for ( int j = 0; j < 3; ++j )
				bTriHasScalars = bTriHasScalars && m_pMesh->GetScalar( vTri[j], 0, vTriScalar[j] );
		}

		for ( int j = 0 ; j < 3; ++j ) {
			if ( m_eNormalMode == VertexNormals )
				glNormal3fv( vNorm[j] );
			else if ( m_eNormalMode == FaceNormals ) {
				Wml::Vector3f vEdge1(vVtx[1] - vVtx[0]);
				Wml::Vector3f vEdge2(vVtx[2] - vVtx[0]);
				Wml::Vector3f vFaceNormal( vEdge1.Cross(vEdge2) );
				vFaceNormal.Normalize();
				glNormal3fv(vFaceNormal);
			}

			if ( m_eColorMode == VertexColors ) {
				m_pMesh->GetColor( vTri[j], cColor );
				glColor4fv( cColor );
			}
			if ( m_eTexture2DMode == VertexTexture2D || m_eTexture2DMode == VertexTexture2D_Required )
				glTexCoord2fv( pCurUV[j] );

			if ( bShowScalars && bTriHasScalars ) {
				float fV = vTriScalar[j];

				// hack for per-triangle scalar renderering
				//if ( vTriScalar[0] == 1 || vTriScalar[1] == 1 || vTriScalar[2] == 1 )
				//	fV = 1;
				//else if ( vTriScalar[0] > 1 || vTriScalar[1] > 1 || vTriScalar[2] > 1 )
				//	fV = 1.1;
				//if ( fV == 1.0f )
				//	glColor3f( 1, 1, 1 );
				//else if ( fV > 1 )
				//	glColor3f( 0.8f, 0.8f, 1.0f );
				//else
				//	glColor3f( 1.0f, 1.0f-fV, 1.0f-fV);

				// straight mapping
				glColor3f( cColor.R() * fV, cColor.G() * fV, cColor.B() * fV );

				// sin-modulated color for geodesic distance scalar
				//float fMul1 = 45.0f;
				//fV = (1.0f + sin(fMul1 * fV)) / 2.0f;
				//glColor3f( cColor.R() * fV, cColor.G() * fV, cColor.B() * fV );


				// fading color
				//fV = (1.0f + sin(1.0f * fV)) / 2.0f;
				//fV = (float)abs(sin(1.0f * fV));
				//glColor3f( cColor.R() * fV, cColor.G() * fV, cColor.B() * fV );


				// set color for bin
				//int nV = (int)fV;
				//if ( nV == 0 )
				//	glColor3f(0.0f, 0.0f, 0.0f);
				//else if ( nV % 3 == 0 )
				//	glColor3f(1.0f, 0.0f, 0.0f);
				//else if ( nV % 3 == 1 )
				//	glColor3f(0.0f, 1.0f, 0.0f);
				//else
				//	glColor3f(0.0f, 0.0f, 1.0f);

			}

			glVertex3fv( vVtx[j] );
		}

	}
	glEnd();


	if ( m_bDrawNormals )
		RenderNormals();

	glPopAttrib();

//	DrawBoundaryEdges();
}

void VFMeshRenderer::RenderNormals()
{
	float fScaleFactor = 0.025f;

	Wml::Vector3f vVtx[3], vNorm[3];

	glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT);
	glDisable(GL_LIGHTING);
	glLineWidth(2.0f);

	glColor4f(0.5f, 0.5f, 0.5f, 1.0f);

	glBegin(GL_LINES);
	VFTriangleMesh::triangle_iterator 
		cur( m_pMesh->BeginTriangles() ),
		end( m_pMesh->EndTriangles() );
	while ( cur != end ) {
		IMesh::TriangleID nTriID = *cur;	cur++;
		m_pMesh->GetTriangle( nTriID, vVtx, vNorm );

		for ( int j = 0 ; j < 3; ++j ) {
			glVertex3fv( vVtx[j] );
			glVertex3fv( vVtx[j] + fScaleFactor * vNorm[j] );
		}

	}
	glEnd();

	glPopAttrib();
}





void VFMeshRenderer::Render_UV(bool bWithNormals)
{
	Wml::Vector2f vTriUV[3];
	Wml::Vector2f vInvalidUV[3];
	Wml::Vector3f vVerts[3], vNormals[3];
	vInvalidUV[0] = vInvalidUV[1] = vInvalidUV[2] = Wml::Vector2f(99999.0f, 99999.0f);
	Wml::Vector2f * pCurUV;

	VFTriangleMesh::triangle_iterator 
		cur( m_pMesh->BeginTriangles() ),
		end( m_pMesh->EndTriangles() );

	glBegin(GL_TRIANGLES);
	while ( cur != end ) {
		IMesh::TriangleID nTriID = *cur;	cur++;

		pCurUV = ( m_pMesh->GetTriangleUV( nTriID, 0, vTriUV )) ? vTriUV : vInvalidUV;
		if ( pCurUV == vInvalidUV )
			continue;

		if ( bWithNormals )
			m_pMesh->GetTriangle(nTriID, vVerts, vNormals);

		for ( int j = 0 ; j < 3; ++j ) {
			if ( bWithNormals )
				glNormal3fv( vNormals[j] );
			glTexCoord2fv( pCurUV[j] );
			glVertex2fv( pCurUV[j] );
		}

	}
	glEnd();
}



void VFMeshRenderer::DrawBoundaryEdges()
{
	glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT);
	glDisable(GL_LIGHTING);
	glLineWidth(4.0f);

	glColor4f(1.0f,0.0f,0.0f,1.0f);

	glBegin(GL_LINES);

	rms::IMesh::TriangleID nNbr[3];
	Wml::Vector3f vTri[3];
	rms::VFTriangleMesh::triangle_iterator curt(m_pMesh->BeginTriangles()), endt(m_pMesh->EndTriangles());
	while ( curt != endt ) {
		rms::IMesh::TriangleID tID = *curt; ++curt;
		m_pMesh->FindNeighbours(tID, nNbr);
		m_pMesh->GetTriangle(tID, vTri);
		for ( int j = 0; j < 3; ++j ) {
			if ( nNbr[j] == rms::IMesh::InvalidID ) {
				glVertex3fv( vTri[j] );
				glVertex3fv( vTri[ (j+1) % 3 ] );
			}
		}
	}
	glEnd();

	glPopAttrib();
}





