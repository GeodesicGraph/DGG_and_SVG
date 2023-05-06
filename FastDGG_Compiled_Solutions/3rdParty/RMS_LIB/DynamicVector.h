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
#ifndef _RMS_DYNAMIC_VECTOR_H
#define _RMS_DYNAMIC_VECTOR_H

#include <vector>

namespace rms
{


template<class Type>
class DataSegment {
public:
	Type * pData;
	unsigned int nSize;
	unsigned int nCur;
	DataSegment() { pData = NULL; }
	~DataSegment() { }
};


template<class Type>
class DynamicVector
{
public:
	DynamicVector(unsigned int nSegmentSize = 0);
	virtual ~DynamicVector();

	void clear( bool bFreeSegments = false );
	void resize( unsigned int nCount );

	size_t size() const;

	void push_back( const Type & data );
	Type * push_back();

	Type & operator[]( unsigned int nIndex );
	const Type & operator[]( unsigned int nIndex ) const;

protected:
	unsigned int m_nSegmentSize;

	unsigned int m_nCurSeg;

	std::vector< DataSegment<Type> > m_vSegments;

	Type * allocate_element();
};




template <class Type>
DynamicVector<Type>::DynamicVector(unsigned int nSegmentSize)
{
	if ( nSegmentSize == 0 )
		m_nSegmentSize = (1 << 16) / sizeof(Type);		// 64k
	else
		m_nSegmentSize = nSegmentSize;

	m_vSegments.resize(1);
	m_vSegments[0].pData = new Type[ m_nSegmentSize ];
	m_vSegments[0].nSize = m_nSegmentSize;
	m_vSegments[0].nCur = 0;

	m_nCurSeg = 0;
}

template <class Type>
DynamicVector<Type>::~DynamicVector()
{
	size_t nCount = m_vSegments.size();
	for (unsigned int i = 0; i < nCount; ++i)
		delete [] m_vSegments[i].pData;
}


template <class Type>
void DynamicVector<Type>::clear( bool bFreeSegments )
{
	size_t nCount = m_vSegments.size();
	for (unsigned int i = 0; i < nCount; ++i) 
		m_vSegments[i].nCur = 0;

	if (bFreeSegments) {
		for (unsigned int i = 1; i < nCount; ++i)
			delete [] m_vSegments[i].pData;
		m_vSegments.resize(1);
	}

	m_nCurSeg = 0;
}


template <class Type>
void DynamicVector<Type>::resize( unsigned int nCount )
{
	// figure out how many segments we need
	unsigned int nNumSegs = 1 + nCount / m_nSegmentSize;

	// figure out how many are currently allocated...
	size_t nCurCount = m_vSegments.size();

	// erase extra segments memory
	for ( unsigned int i = nNumSegs; i < nCurCount; ++i )
		delete [] m_vSegments[i].pData;

	// resize to right number of segments
	m_vSegments.resize(nNumSegs);

	// allocate new segments
	for (unsigned int i = (unsigned int)nCurCount; i < nNumSegs; ++i) {
		m_vSegments[i].pData = new Type[ m_nSegmentSize ];
		m_vSegments[i].nSize = m_nSegmentSize;
		m_vSegments[i].nCur = 0;
	}

	// mark full segments as used
	for (unsigned int i = 0; i < nNumSegs-1; ++i)
		m_vSegments[i-1].nCur = m_nSegmentSize;

	// mark last segment
	m_vSegments[nNumSegs-1].nCur = nCount - (nNumSegs-1)*m_nSegmentSize;

	m_nCurSeg = nNumSegs-1;
}


template <class Type>
size_t  DynamicVector<Type>::size() const
{
	return (m_nCurSeg)*m_nSegmentSize + m_vSegments[m_nCurSeg].nCur;
}


template <class Type>
Type * DynamicVector<Type>::allocate_element()
{
	DataSegment<Type> & seg = m_vSegments[m_nCurSeg];
	if ( seg.nCur == seg.nSize ) {
		if ( m_nCurSeg == m_vSegments.size() - 1 ) {
			m_vSegments.resize( m_vSegments.size() + 1 );
			DataSegment<Type> & newSeg = m_vSegments.back();
			newSeg.pData = new Type[ m_nSegmentSize ];
			newSeg.nSize = m_nSegmentSize;
			newSeg.nCur = 0;
		}
		m_nCurSeg++;
	}
	DataSegment<Type> & returnSeg = m_vSegments[m_nCurSeg];
	return & returnSeg.pData[ returnSeg.nCur++ ];
}

template <class Type>
void DynamicVector<Type>::push_back( const Type & data )
{
	Type * pNewElem = allocate_element();
	*pNewElem = data;
}

template <class Type>
Type * DynamicVector<Type>::push_back()
{
	return allocate_element();
}


template <class Type>
Type & DynamicVector<Type>::operator[]( unsigned int nIndex )
{
	return m_vSegments[ nIndex / m_nSegmentSize ].pData[ nIndex % m_nSegmentSize ];
}

template <class Type>
const Type & DynamicVector<Type>::operator[]( unsigned int nIndex ) const
{
	return m_vSegments[ nIndex / m_nSegmentSize ].pData[ nIndex % m_nSegmentSize ];
}


} // end namespace rms


#endif _RMS_DYNAMIC_VECTOR_H