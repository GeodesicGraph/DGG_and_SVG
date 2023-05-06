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
#ifndef __RMS_SPARSE_ARRAY_H__
#define __RMS_SPARSE_ARRAY_H__

// ignore annoying warning about dll-interface for vector that is not exposed...
#pragma warning( push )
#pragma warning( disable: 4251 )

#include <set>
#include "Wm4Vector3.h"
#include "Wm4Vector2.h"
#include "MemoryPool.h"

namespace rms {


// RMS TODO: replace std::set with a balanced binary tree (can easily
//  be balanced if we know bucket size). Do allocation via a mempool
//  shared with SparseArray...

template<class Type>
class SparseBucket
{

public:  // Entry should pobably not be public, but it avoids having too write a custom iterator class...
	typedef unsigned int Index;
	struct Entry {
		Index i;
		Type val;
		inline bool operator<( const Entry & e2 ) const
			{ return i < e2.i; }
		inline bool operator==( const Entry & e2 ) const
			{ return i == e2.i; }
	};
	std::set< Entry > m_vData;

public:

	inline void clear()
		{ m_vData.clear(); }

	inline bool empty() const
		{ return m_vData.empty(); }

	inline bool set( Index i, const Type & v ) { 
		Entry e; e.i = i; e.val = v; 
		std::pair< std::set<Entry>::iterator, bool > pr = m_vData.insert(e);
		if ( ! pr.second ){
            assert(false);
			//(*pr.first).val = v;
        }
		return pr.second;
	}

	inline bool remove( Index i ) {
		Entry e; e.i = i; 
		std::pair< std::set<Entry>::iterator, bool > pr = m_vData.erase(e); 
		return pr.second;
	}
		

	inline bool has( Index i ) const {
		Entry e; e.i = i;
		std::set<Entry>::const_iterator cur( m_vData.find(e) );
		return (cur != m_vData.end());
	}

	inline Type & get( Index i ) { 
		Entry e; e.i = i;
		std::set<Entry>::iterator cur( m_vData.find(e) );
		return (*cur).val;
	}
	inline const Type & get( Index i ) const { 
		Entry e; e.i = i;
		std::set<Entry>::const_iterator cur( m_vData.find(e) );
		return (*cur).val;
	}

	inline Type & operator[]( Index i ) {
		Entry e; e.i = i;
		std::set<Entry>::iterator cur( m_vData.find(e) );
		return (*cur).val;
	}
	inline const Type & operator[]( Index i ) const {
		Entry e; e.i = i;
		std::set<Entry>::iterator cur( m_vData.find(e) );
		return (*cur).val;
	}

	typedef typename std::set<Entry>::iterator iterator;

	inline iterator begin() { return m_vData.begin(); }
	inline iterator end() { return m_vData.end(); }
};


// [RMS: bucket size is currently hardcoded. Should probably be a variable...]
#define BUCKET_POW2 10
#define BUCKET_SIZE		(1 << BUCKET_POW2)
#define BUCKET_INDEX(k)  ( (k) >> BUCKET_POW2 )
#define BUCKET_MASK(k) ( (k) & 0x3FF )

template<class Type>
class SparseArray
{
public:
	typedef unsigned int Index;

	SparseArray( unsigned int nSize = 0 )
		{ resize(nSize); m_nCount = 0; }

	inline void clear( bool bFreeBuckets = true ) { 
		for ( unsigned int i = 0; i < m_vBuckets.size(); ++i ) 
			m_vBuckets[i].clear();
		m_nCount = 0;
		if ( bFreeBuckets )
			m_vBuckets.clear();
	}

	inline void resize( unsigned int nSize )
		{ m_vBuckets.resize(nSize / BUCKET_SIZE + ((nSize % BUCKET_SIZE == 0) ? 0 : 1) ); }

	inline size_t size() const
		{ return m_nCount; }

	inline bool empty() const
		{ return m_nCount == 0; }

	inline void set( Index i, const Type & v ) 
		{	assert( BUCKET_INDEX(i) < m_vBuckets.size() );
			if ( m_vBuckets[ BUCKET_INDEX(i) ].set( BUCKET_MASK(i), v ) ) ++m_nCount; }

	inline void erase( Index i ) 
		{ assert( BUCKET_INDEX(i) < m_vBuckets.size() );
		  if ( m_vBuckets[ BUCKET_INDEX(i) ].erase( BUCKET_MASK(i) ) ) --m_nCount; }

	inline Type & get( Index i )
		{	assert( BUCKET_INDEX(i) < m_vBuckets.size() );
			return m_vBuckets[ BUCKET_INDEX(i) ].get( BUCKET_MASK(i) ); }
	inline const Type & get( Index i ) const
		{	assert( BUCKET_INDEX(i) < m_vBuckets.size() );
			return m_vBuckets[ BUCKET_INDEX(i) ].get( BUCKET_MASK(i) ); }

	inline bool has( Index i ) const
		{	assert( BUCKET_INDEX(i) < m_vBuckets.size() );
			return BUCKET_INDEX(i) < m_vBuckets.size() && m_vBuckets[ BUCKET_INDEX(i) ].has( BUCKET_MASK(i) ); }

	inline Type & operator[]( Index i ) 
		{	assert( BUCKET_INDEX(i) < m_vBuckets.size() );
			return m_vBuckets[ BUCKET_INDEX(i) ].get( BUCKET_MASK(i) ); }
	inline const Type & operator[]( Index i ) const
		{	assert( BUCKET_INDEX(i) < m_vBuckets.size() );
			return m_vBuckets[ BUCKET_INDEX(i) ].get( BUCKET_MASK(i) ); }


	/*
	 * iterators
	 */
	class iterator {
	public:
		inline iterator( const iterator & i2 ) {
			m_pArray = i2.m_pArray;
			m_nCurBucket = i2.m_nCurBucket;
			m_bcur = i2.m_bcur;
		}

		inline Type & operator*() { return (*m_bcur).val; }

		inline iterator & operator++() {		// prefix
			goto_next();
			return *this;
		}
		inline iterator operator++(int) {		// postfix
			iterator copy(*this);
			goto_next();
			return copy;
		}

		inline bool operator==( const iterator & i2 ) {
			return ( m_pArray == i2.m_pArray &&  m_nCurBucket == i2.m_nCurBucket && m_bcur == i2.m_bcur );			// array & bucket must be the same for this to occur!
		}
		inline bool operator!=( const iterator & i2 ) {
			return ( m_pArray != i2.m_pArray ||  m_nCurBucket != i2.m_nCurBucket || m_bcur != i2.m_bcur );			// array & bucket must be the same for this to occur!
		}

		inline Index index() {
			return (m_nCurBucket * BUCKET_SIZE) + (*m_bcur).i;
		}


	protected:
		SparseArray<Type> * m_pArray;
		Index m_nCurBucket;
		typename SparseBucket<Type>::iterator m_bcur;
		friend class SparseArray<Type>;

		inline iterator( SparseArray<Type> * pArray, bool bStart ) {
			m_pArray = pArray;
			if ( bStart ) {
				m_nCurBucket = 0;
				m_bcur = m_pArray->m_vBuckets[0].begin();
				if ( m_bcur == m_pArray->m_vBuckets[0].end() )
					goto_next();
			} else {
				m_nCurBucket = (m_pArray->m_vBuckets.size() > 0) ? 
					(Index)(m_pArray->m_vBuckets.size()-1) : 0;
				m_bcur = m_pArray->m_vBuckets[m_nCurBucket].end();
			}
		}

		inline void goto_next() {
			if ( m_bcur == m_pArray->m_vBuckets[m_nCurBucket].end() ) {
				while ( (int)m_nCurBucket < (int)m_pArray->m_vBuckets.size()-1 && 
						m_bcur == m_pArray->m_vBuckets[m_nCurBucket].end() ) {
					m_nCurBucket++;
					m_bcur = m_pArray->m_vBuckets[m_nCurBucket].begin();
				} 
			} else {
				m_bcur++;

				// recursive hack to handle case where m_bcur just hit end(). is
				// there a cleaner way to write this ??
				if ( m_bcur == m_pArray->m_vBuckets[m_nCurBucket].end() )
					goto_next();
			}
		}
	};
	friend class SparseArray<Type>::iterator;

	inline iterator begin() { return iterator(this, true); }
	inline iterator end() { return iterator(this, false); }

protected:
	DynamicVector< SparseBucket<Type> > m_vBuckets;
	unsigned int m_nCount;
};


}


#endif // __RMS_SPARSE_ARRAY_H__