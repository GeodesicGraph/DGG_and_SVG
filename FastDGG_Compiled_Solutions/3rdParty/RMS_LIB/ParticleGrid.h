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
#ifndef _RMS_PARTICLE_GRID_H_
#define _RMS_PARTICLE_GRID_H_

#include <vector>
#include <map>

#include "MemoryPool.h"

namespace rms {


template<class Type>
class ParticleGrid
{
public:
	ParticleGrid() {};
	~ParticleGrid() {};

	void Initialize( const float * vOrigin, float fCellSize );

	float CellSize() { return m_fCellSize; }

	void AddParticle( Type pParticle, const float * vPosition );
	void Clear();


	class BoxIterator {
	public:
		BoxIterator( ParticleGrid * pGrid, const float * vPosition, float fRadius );
		~BoxIterator() {}

		bool Done() { return m_nCur == m_nStop; }

		Type & operator *() 
			{ return m_pCurVoxel->particles[m_nVoxelCur]; }
		void operator++();
		void operator++(int nPostfix) { this->operator++(); }
		bool operator==( const BoxIterator & p2 ) const 
			{ return m_nCur == p2.m_nCur && m_nVoxelCur == p2.m_nVoxelCur; }
		bool operator!=( const BoxIterator & p2 ) const
			{ return ! (m_nCur == p2.m_nCur && m_nVoxelCur == p2.m_nVoxelCur); }

	protected:
		ParticleGrid * m_pGrid;
		typename ParticleGrid::Voxel * m_pCurVoxel;
		unsigned int m_nVoxelCount;
		unsigned int m_nVoxelCur;

		int m_nStart[3];		// x,y,z of start cell
		int m_nDims[3];		// x,y,z widths of iteration
		int m_nCur;			// cur index
		int m_nStop;			// stop index
	};
	friend class BoxIterator;

protected:
	float m_vOrigin[3];
	float m_fCellSize;

	struct VoxelKey {
		int x;
		int y;
		int z;

		bool operator<( const VoxelKey & voxel2 ) const {
			return x < voxel2.x || x == voxel2.x && (y < voxel2.y || y == voxel2.y && (z < voxel2.z));
		}
	};

	void GetKey( float fX, float fY, float fZ, VoxelKey & key );

	struct Voxel {
		VoxelKey key;
		std::vector<Type> particles;
	};

	typedef std::map<VoxelKey, Voxel *> VoxelMap;
	VoxelMap m_voxels;

	Voxel * GetVoxel( float fX, float fY, float fZ, bool bCreate = false );
	Voxel * GetVoxel( const VoxelKey & key, bool bCreate = false );

	MemoryPool<Voxel> m_vVoxelMemoryPool;
};



template<class Type>
void ParticleGrid<Type>::Initialize( const float * vOrigin, float fCellSize )
{
	m_vOrigin[0] = vOrigin[0]; m_vOrigin[1] = vOrigin[1]; m_vOrigin[2] = vOrigin[2];
	m_fCellSize = fCellSize;
	Clear();
}

template<class Type>
void ParticleGrid<Type>::AddParticle( Type pParticle, const float * vPosition )
{
	// get voxel
	Voxel * pVoxel = GetVoxel( vPosition[0], vPosition[1], vPosition[2], true );

	pVoxel->particles.push_back(pParticle);
}

template<class Type>
void ParticleGrid<Type>::Clear()
{
	m_voxels.clear();
	m_vVoxelMemoryPool.ClearAll();
}

template<class Type>
void ParticleGrid<Type>::GetKey( float fX, float fY, float fZ, VoxelKey & key )
{
	key.x = (int)floorf( (fX - m_vOrigin[0]) / m_fCellSize );
	key.y = (int)floorf( (fY - m_vOrigin[1]) / m_fCellSize );
	key.z = (int)floorf( (fZ - m_vOrigin[2]) / m_fCellSize );
}

template<class Type>
typename ParticleGrid<Type>::Voxel * ParticleGrid<Type>::GetVoxel( float fX, float fY, float fZ, bool bCreate )
{
	VoxelKey key;
	GetKey(fX, fY, fZ, key );
	return GetVoxel( key, bCreate );
}

template<class Type>
typename ParticleGrid<Type>::Voxel *  ParticleGrid<Type>::GetVoxel( typename const ParticleGrid<Type>::VoxelKey & key, bool bCreate )
{
	VoxelMap::iterator found( m_voxels.find(key) );
	if ( found != m_voxels.end() ) {
		return (*found).second;
	} else if ( bCreate ) {
		Voxel * pVoxel = m_vVoxelMemoryPool.Allocate();
		pVoxel->particles.resize(0);
		pVoxel->key = key;
		m_voxels[key] = pVoxel;
		return pVoxel;
	} else 
		return NULL;	
}


template<class Type>
ParticleGrid<Type>::BoxIterator::BoxIterator( ParticleGrid<Type> * pGrid, const float * vPosition, float fRadius )
{
	m_pGrid = pGrid;

	// find bounds
	ParticleGrid<Type>::VoxelKey key, lowKey, highKey;
	m_pGrid->GetKey( vPosition[0], vPosition[1], vPosition[2], key );
	m_pGrid->GetKey( vPosition[0] - fRadius, vPosition[1] - fRadius, vPosition[2] - fRadius, lowKey );
	m_pGrid->GetKey( vPosition[0] + fRadius, vPosition[1] + fRadius, vPosition[2] + fRadius, highKey );

	m_nStart[0] = lowKey.x;
	m_nStart[1] = lowKey.y;
	m_nStart[2] = lowKey.z;

	// find radius dist
	int nDim = (int)ceilf(fRadius / m_pGrid->CellSize());

	// set search dims
	m_nDims[0] = highKey.x - lowKey.x + 1;
	m_nDims[1] = highKey.y - lowKey.y + 1;
	m_nDims[2] = highKey.z - lowKey.z + 1;

	m_nCur = 0;
	m_nStop = m_nDims[0] * m_nDims[1] * m_nDims[2];

	// find starting voxel
	m_pCurVoxel = NULL;
	VoxelKey which;
	while ( m_pCurVoxel == NULL && m_nCur < m_nStop ) {
		int nCur = m_nCur;
		which.z = m_nStart[2] + nCur / (m_nDims[1] * m_nDims[0]);
		nCur %= (m_nDims[1] * m_nDims[0]);
		which.y = m_nStart[1] + nCur / m_nDims[0];
		nCur %= m_nDims[0];
		which.x = m_nStart[0] + nCur;

		m_pCurVoxel = m_pGrid->GetVoxel( which );
		if ( m_pCurVoxel == NULL || m_pCurVoxel->particles.size() == 0 ) {
			m_pCurVoxel = NULL;
			m_nCur++;
		}
	}

	if ( m_nCur < m_nStop ) {
		m_nVoxelCount = (unsigned int)m_pCurVoxel->particles.size();
		m_nVoxelCur = 0;
	}

}


template<class Type>
void ParticleGrid<Type>::BoxIterator::operator++( )
{
	// increment through current vector if possible
	m_nVoxelCur++;
	if ( m_nVoxelCur < m_nVoxelCount ) {
		return;
	}

	// increment cur voxel
	m_nCur++;

	// find next allocated voxel
	m_pCurVoxel = NULL;
	VoxelKey which;
	while ( m_pCurVoxel == NULL && m_nCur < m_nStop ) {
		int nCur = m_nCur;
		which.z = m_nStart[2] + nCur / (m_nDims[1] * m_nDims[0]);
		nCur %= (m_nDims[1] * m_nDims[0]);
		which.y = m_nStart[1] + nCur / m_nDims[0];
		nCur %= m_nDims[0];
		which.x = m_nStart[0] + nCur;

		m_pCurVoxel = m_pGrid->GetVoxel( which );
		if ( m_pCurVoxel == NULL || m_pCurVoxel->particles.size() == 0 ) {
			m_pCurVoxel = NULL;
			m_nCur++;
		}
	}

	if ( m_nCur < m_nStop ) {
		m_nVoxelCount = (unsigned int)m_pCurVoxel->particles.size();
		m_nVoxelCur = 0;
	}	
}


} // namespace rmsexpmap

#endif _RMS_PARTICLE_GRID_H_