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
#ifndef _RMSUI_TEXTURE_H_
#define _RMSUI_TEXTURE_H_

#include <vector>
#include <map>
#include <set>
#include <gl/GL.h>


// this is an OpenGL texture class
class Texture 
{
public:
	typedef unsigned int TextureID;
	static TextureID INVALID_TEXTURE;

	Texture(TextureID nID = INVALID_TEXTURE);
	Texture( const Texture * pCopy );
	~Texture();

	// set texture image data. Calling this will free any existing texture!
	void InitializeBuffer( unsigned int nWidth, unsigned int nHeight, 
						   const std::vector<unsigned char> & vBuffer,
						   unsigned int nGLTextureFormat );

	// set texture flags
	void SetTextureInfo( unsigned int nGLTextureType,
						 unsigned int nGLTextureFilter = GL_LINEAR,
						 unsigned int nGLTextureWrapMode = GL_CLAMP );


	// [RMS: this function performs texture initialization...do not call before OpenGL is enabled!]
	unsigned int GLTextureID();

	TextureID ID() { return m_nTextureID; }

	unsigned int Width() { return m_nWidth; }
	unsigned int Height() { return m_nHeight; }
	unsigned int Channels() { return m_nComponents; }
	const std::vector<unsigned char> & Buffer() {  return m_vBuffer; }

	unsigned int & TextureType() { return m_nGLTextureType; }
	unsigned int & TextureEnvMode()  { return m_nGLTextureEnvMode; }

	void Enable();
	void Disable();

	// releases texture
	void FreeTexture();


	// utility stuff

	void ReplaceColorAlpha( unsigned char nRed, unsigned char nBlue, unsigned char nGreen, unsigned char nNewAlpha );

	void SetEdgeAlpha( unsigned char nSetAlpha  );

	void FlipY();

	static void MakeCheckerImage( int nTexSize, int nCheckerSize, std::vector<unsigned char> & vTexBuf, bool bAlpha = false );

	// buffer rgb is channel map [0,1,2], buffer bgr is channel map [2,1,0]
	static void MakeFromBuffer( unsigned int nImageWidth, unsigned int nImageHeight, unsigned int nImageChannels,
								const unsigned char * pBuffer,
							    const unsigned int nChannelMap[3], bool bAddAlpha, 
								std::vector<unsigned char> & vTexBuf, unsigned int & nTexWidth, unsigned int & nTexHeight );

	const unsigned char * GetPixel( unsigned int x, unsigned int y );
	const unsigned char * GetPixel( float fUnitX, float fUnitY );
	void GetTexCoords( float fUnitX, float fUnitY, unsigned int & xi, unsigned int & yi );

protected:
	static TextureID m_sTextureIDGen;
	static TextureID GenTextureID();

	TextureID m_nTextureID;
	unsigned int m_nOpenGLTextureID;
	unsigned int m_nDeleteThisOpenGLTextureID;

	unsigned int m_nWidth;
	unsigned int m_nHeight;
	unsigned int m_nComponents;
	std::vector<unsigned char> m_vBuffer;

	unsigned int m_nTexWidth;
	unsigned int m_nTexHeight;

	float m_fScaleX;
	float m_fScaleY;

	unsigned int m_nGLTextureFormat;	// GL_RGB / RGBA / etc
	unsigned int m_nGLTextureType;		// GL_TEXURE_1D / 2D / 3D
	unsigned int m_nGLTextureEnvMode;	// GL_MODULATE / DECAL / REPLACE / etc
	unsigned int m_nGLTextureFilter;	// GL_NEAREST / GL_LINEAR
	unsigned int m_nGLTextureWrapMode;	// GL_REPEAT / GL_CLAMP
};

class TextureSet
{
public:
	TextureSet();

	void AddTexture( Texture * pTexture );
	void RemoveTexture( Texture * pTexture );

	Texture * GetTexture( Texture::TextureID nID );

	unsigned int GetTextureCount() { return (unsigned int)m_vOrderedList.size(); }
	Texture * GetTextureByIndex( unsigned int nIndex );

protected:
	std::map<Texture::TextureID, Texture *> m_vTextures;

	std::set< Texture::TextureID > m_vOrderedList;
};



#endif // _RMSUI_TEXTURE_H_