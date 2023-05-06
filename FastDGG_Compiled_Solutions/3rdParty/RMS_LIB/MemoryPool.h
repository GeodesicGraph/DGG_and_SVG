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
#ifndef _RMS_MEMORY_POOL_H
#define _RMS_MEMORY_POOL_H

// ignore annoying warning about dll-interface for vector that is not exposed...
#pragma warning( disable: 4251 )

#include <vector>
#include "DynamicVector.h"


namespace rms
{

template<class Type>
class MemoryPool
{
public:
	MemoryPool();
	~MemoryPool();

	Type * Allocate();
	void Free(Type * pData);

	void ClearAll();

protected:
	DynamicVector<Type> m_vStore;

	std::vector<Type *> m_vFree;
};


template<class Type>
MemoryPool<Type>::MemoryPool()
{
}

template<class Type>
MemoryPool<Type>::~MemoryPool()
{
}

template<class Type>
Type * MemoryPool<Type>::Allocate()
{
	if ( m_vFree.empty() )
		return m_vStore.push_back();
	else {
		Type * pType = m_vFree.back();
		m_vFree.pop_back();
		return pType;
	}
}

template<class Type>
void MemoryPool<Type>::Free(Type * pData)
{
	m_vFree.push_back( pData );
}

template<class Type>
void MemoryPool<Type>::ClearAll()
{
	m_vFree.clear();
	m_vStore.clear();
}


} // end namespace rms;


#endif   _RMS_MEMORY_POOL_H