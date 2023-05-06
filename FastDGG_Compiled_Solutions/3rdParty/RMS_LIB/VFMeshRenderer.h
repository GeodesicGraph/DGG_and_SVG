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
#ifndef __RMS_VFMESH_RENDERER_H__
#define __RMS_VFMESH_RENDERER_H__

#include "VFTriangleMesh.h"

namespace rms {

class VFMeshRenderer
{
public:
	VFMeshRenderer( VFTriangleMesh * pMesh );
	~VFMeshRenderer(void);

	void Render();
	void Render_UV(bool bWithNormals);

	enum NormalMode {
		NoNormals,
		VertexNormals,
		FaceNormals
	};
	NormalMode GetNormalMode() { return m_eNormalMode; }
	void SetNormalMode( NormalMode eMode ) { m_eNormalMode = eMode; }

	enum ColorMode {
		NoColors,
		VertexColors
	};
	ColorMode GetColorMode() { return m_eColorMode; }
	void SetColorMode( ColorMode eMode ) { m_eColorMode = eMode; }


	enum Texture2DMode {
		NoTexture2D,
		VertexTexture2D,
		FaceTexture2D,
		VertexTexture2D_Required
	};
	Texture2DMode GetTexture2DMode() { return m_eTexture2DMode; }
	void SetTexture2DMode( Texture2DMode eMode ) { m_eTexture2DMode = eMode; }

	void SetModes( NormalMode eNormalMode, Texture2DMode eTex2DMode ) 
		{ m_eNormalMode = eNormalMode; m_eTexture2DMode = eTex2DMode; }

	void SetDrawNormals( bool bDrawNormals ) { m_bDrawNormals = bDrawNormals; }
	bool GetDrawNormals() { return m_bDrawNormals; }

	void SetEnableScalarColor( bool bEnable ) { m_bEnableScalarColor = bEnable; }
	bool GetEnableScalarColor() { return m_bEnableScalarColor; }

protected:
	VFTriangleMesh * m_pMesh;

	NormalMode m_eNormalMode;
	ColorMode m_eColorMode;
	Texture2DMode m_eTexture2DMode;

	bool m_bDrawNormals;
	void RenderNormals();

	bool m_bEnableScalarColor;

	void DrawBoundaryEdges();
};




} // end namespace rmsmesh


#endif // __RMS_VFMESH_RENDERER_H__