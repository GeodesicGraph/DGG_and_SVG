// CBaseModel.h: interface for the CBaseModel class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include "Point3D.h"

class CBaseModel  
{
public:	
	CBaseModel(const string& filename);
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
	void FastReadObjFile(const string& filename);
	void ReadMFile(const string& filename);
	void ReadObjFile(const string& filename);
	void ReadOffFile(const string& filename);
    
public:
	void AdjustScaleAndComputeNormalsToVerts();
	void ReadFile(const string& filename);
	void FastSaveObjFile(const string& filename, const vector<pair<double, double>>& texture) const;
	void FastSaveObjFile(const string& filename) const;
    void FastSaveObjFileOfGivenFaces(const string& filename,const set<int>& faces) const;
    void FastSaveObjFileWithNormal(const string& filename) const;
    void Scale(double scale);
	void Translate(const CPoint3D& t);
	inline int GetNumOfVerts() const;
	inline int GetNumOfFaces() const;
	void LoadModel();
	inline const CPoint3D& Vert(int vertIndex) const;
	inline const CPoint3D& Normal(int vertIndex) const;
	inline const CFace& Face(int faceIndex) const;

	string GetFullPathAndFileName() const;

	virtual void Render(GLenum mode) const;
	virtual void Render(GLenum mode, const vector<float>* distance_filed) const;
	void DrawWireframe() const;
	void DrawPoint() const;
public:
	vector<CPoint3D> m_Verts;
    vector<CPoint3D> m_vertex_colors;
	vector<CFace> m_Faces;
	vector<CPoint3D> m_NormalsToVerts;
	bool m_fBeLoaded;
public:
	string m_filename;
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


