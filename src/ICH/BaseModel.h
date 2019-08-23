// CBaseModel.h: interface for the CBaseModel class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include "Point3D.h"

class CBaseModel  
{
public:	
	CBaseModel(const std::string& filename);
public:
	struct CFace
	{
		int verts[3];
		int edges[3];
		double maxEdgeLen;
		CFace(){}
		CFace(int x, int y, int z)
		{
			verts[0] = x;
			verts[1] = y;
			verts[2] = z;
		}
		int& operator[](int index)
		{
			return verts[index];
		}
		int operator[](int index) const
		{
			return verts[index];
		} 
	};
	
protected:	
	void FastReadObjFile(const std::string& filename);
	void ReadMFile(const std::string& filename);
	void ReadObjFile(const std::string& filename);
	void ReadOffFile(const std::string& filename);
    
public:
	void AdjustScaleAndComputeNormalsToVerts();
	void ReadFile(const std::string& filename);
	void FastSaveObjFile(const std::string& filename, const std::vector<std::pair<double, double>>& texture) const;
	void FastSaveObjFile(const std::string& filename) const;
    void FastSaveObjFileOfGivenFaces(const std::string& filename,const std::set<int>& faces) const;
    void FastSaveObjFileWithNormal(const std::string& filename) const;
    void Scale(double scale);
	void Translate(const CPoint3D& t);
	inline int GetNumOfVerts() const;
	inline int GetNumOfFaces() const;
	void LoadModel();
	inline const CPoint3D& Vert(int vertIndex) const;
	inline const CPoint3D& Normal(int vertIndex) const;
	inline const CFace& Face(int faceIndex) const;

	std::string GetFullPathAndFileName() const;


public:
	std::vector<CPoint3D> m_Verts;
	std::vector<CPoint3D> m_vertex_colors;
	std::vector<CFace> m_Faces;
	std::vector<CPoint3D> m_NormalsToVerts;
	bool m_fBeLoaded;
public:
	std::string m_filename;
	CPoint3D m_center;
	double m_scale;
	CPoint3D m_ptUp;
	CPoint3D m_ptDown;
};

int CBaseModel::GetNumOfVerts() const
{
	return (int)m_Verts.size();
}

int CBaseModel::GetNumOfFaces() const
{
	return (int)m_Faces.size();
}

const CPoint3D& CBaseModel::Vert(int vertIndex) const
{
	return m_Verts[vertIndex];
}

const CPoint3D& CBaseModel::Normal(int vertIndex) const
{
	return m_NormalsToVerts[vertIndex];
}

const CBaseModel::CFace& CBaseModel::Face(int faceIndex) const
{
	return m_Faces[faceIndex];
}


