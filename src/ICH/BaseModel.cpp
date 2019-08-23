// ObjModel.cpp: implementation of the CBaseModel class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BaseModel.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CBaseModel::CBaseModel(const string& filename) : m_filename(filename)
{
	m_fBeLoaded = false;
}

void CBaseModel::AdjustScaleAndComputeNormalsToVerts()
{
	if (m_Verts.empty())
		return;
	m_NormalsToVerts.resize(m_Verts.size(), CPoint3D(0, 0, 0));
	CPoint3D center(0, 0, 0);
	double sumArea(0);
	CPoint3D sumNormal(0, 0, 0);
	double deta(0);
	for (int i = 0; i < (int)m_Faces.size(); ++i)
	{
		CPoint3D normal = VectorCross(Vert(Face(i)[0]),
			Vert(Face(i)[1]),
			Vert(Face(i)[2]));
		double area = normal.Len();
		CPoint3D gravity3 = Vert(Face(i)[0]) +	Vert(Face(i)[1]) + Vert(Face(i)[2]);
		center += area * gravity3;
		sumArea += area;
		sumNormal += normal;
		deta += gravity3 ^ normal;
		normal.x /= area;
		normal.y /= area;
		normal.z /= area;
		for (int j = 0; j < 3; ++j)
		{
			m_NormalsToVerts[Face(i)[j]] += normal;
		}
	}
	center /= sumArea * 3;
    //printf("center %lf %lf %lf\n" , center.x , center.y , center.z );
	deta -= 3 * (center ^ sumNormal);
	if (true)//deta > 0)
	{
		for (int i = 0; i < GetNumOfVerts(); ++i)
		{
			if (fabs(m_NormalsToVerts[i].x)
				+ fabs(m_NormalsToVerts[i].y)
				+ fabs(m_NormalsToVerts[i].z) >= FLT_EPSILON)
			{					
				m_NormalsToVerts[i].Normalize();
			}
		}
	}
	else
	{
		for (int i = 0; i < GetNumOfFaces(); ++i)
		{
			int temp = m_Faces[i][0];
			m_Faces[i][0] = m_Faces[i][1];
			m_Faces[i][1] = temp;
		}
		for (int i = 0; i < GetNumOfVerts(); ++i)
		{
			if (fabs(m_NormalsToVerts[i].x)
				+ fabs(m_NormalsToVerts[i].y)
				+ fabs(m_NormalsToVerts[i].z) >= FLT_EPSILON)
			{					
				double len = m_NormalsToVerts[i].Len();
				m_NormalsToVerts[i].x /= -len;
				m_NormalsToVerts[i].y /= -len;
				m_NormalsToVerts[i].z /= -len;
			}
		}
	}

	CPoint3D ptUp(m_Verts[0]);
	CPoint3D ptDown(m_Verts[0]);
	for (int i = 1; i < GetNumOfVerts(); ++i)
	{
		if (m_Verts[i].x > ptUp.x)
			ptUp.x = m_Verts[i].x;
		else if (m_Verts[i].x < ptDown.x)
			ptDown.x = m_Verts[i].x;
		if (m_Verts[i].y > ptUp.y)
			ptUp.y = m_Verts[i].y;
		else if (m_Verts[i].y < ptDown.y)
			ptDown.y = m_Verts[i].y;
		if (m_Verts[i].z > ptUp.z)
			ptUp.z = m_Verts[i].z;
		else if (m_Verts[i].z < ptDown.z)
			ptDown.z = m_Verts[i].z;
	}	

	double maxEdgeLenOfBoundingBox = -1;
	if (ptUp.x - ptDown.x > maxEdgeLenOfBoundingBox)
		maxEdgeLenOfBoundingBox = ptUp.x - ptDown.x;
	if (ptUp.y - ptDown.y > maxEdgeLenOfBoundingBox)
		maxEdgeLenOfBoundingBox = ptUp.y - ptDown.y;
	if (ptUp.z - ptDown.z > maxEdgeLenOfBoundingBox)
		maxEdgeLenOfBoundingBox = ptUp.z - ptDown.z;
	m_scale = 2.0 / maxEdgeLenOfBoundingBox;
	m_center = center;
	m_ptUp = ptUp;
	m_ptDown = ptDown;

	m_ptUp = (m_ptUp - center) * m_scale;
	m_ptDown = (m_ptUp - m_ptDown) * m_scale;
	//for (int i = 0; i < (int)m_Verts.size(); ++i)
	//{
	//	m_Verts[i] = (m_Verts[i] - center) * m_scale;
	//}

	m_scale = 1;
	m_center = CPoint3D(0, 0, 0);
}

void CBaseModel::LoadModel()
{
	if (m_fBeLoaded)
		return;
	ReadFile(m_filename);
	AdjustScaleAndComputeNormalsToVerts();
	m_fBeLoaded = true;
}

void CBaseModel::ReadFile(const string& filename)
{
	int nDot = (int)filename.rfind('.');
	if (nDot == -1)
	{
		throw "File name doesn't contain a dot!";
	}
	string extension = filename.substr(nDot + 1);

	if (extension == "obj")
	{
		//ReadObjFile(filename);
		FastReadObjFile(filename);
	}
	else if (extension == "off")
	{
		ReadOffFile(filename);
	}
	else if (extension == "m")
	{
		ReadMFile(filename);
	}
	else
	{
		throw "This format can't be handled!";
	}
}

void CBaseModel::ReadObjFile(const string& filename)
{
	ifstream in(filename.c_str());
	if (in.fail())
	{
		throw "fail to read file";
	}
	char buf[256];
	while (in.getline(buf, sizeof buf))
	{
		istringstream line(buf);
		string word;
		line >> word;
		if (word == "v")
		{
			CPoint3D pt;
			line >> pt.x;
			line >> pt.y;
			line >> pt.z;

			m_Verts.push_back(pt);
		}
		else if (word == "f")
		{
			CFace face;
			int tmp;
			vector<int> polygon;
			polygon.reserve(4);
			while (line >> tmp)
			{
				polygon.push_back(tmp);
				char tmpBuf[256];
				line.getline(tmpBuf, sizeof tmpBuf, ' ');
			}
			for (int j = 1; j < (int)polygon.size() - 1; ++j)
			{
				face[0] = polygon[0] - 1;
				face[1] = polygon[j] - 1;
				face[2] = polygon[j + 1] - 1;
				m_Faces.push_back(face);
			}
		}
		else
		{
			continue;
		}
	}
	m_Verts.swap(vector<CPoint3D>(m_Verts));
	m_Faces.swap(vector<CFace>(m_Faces));
	//printf("file %s num of verts %d num of faces %d\n" , filename.c_str() , m_Verts.size(),m_Faces.size());
	in.close();
}
void CBaseModel::FastReadObjFile(const string& filename)
{
	FILE* fin = fopen( filename.c_str() , "r");
	if( fin == NULL ){
		throw "fail to read file";
	}
	char buf[256];
	while(fgets(buf,256,fin) != NULL){
		char * pch;
		pch = strtok (buf," ");
		//if (pch[0] == 'v'){
		if(strcmp(pch,"v")==0){
			CPoint3D pt;
			pch = strtok (NULL, " ");
			sscanf( pch , "%lf"  ,&pt.x);
			pch = strtok (NULL, " ");
			sscanf( pch , "%lf"  ,&pt.y);
			pch = strtok (NULL, " ");
			sscanf( pch , "%lf"  ,&pt.z);
			m_Verts.push_back(pt);
			//}else if (pch[0] == 'f'){
		}else if(strcmp(pch,"f")==0){
			CFace face;
			for(int j = 0; j < 3; ++j){
				pch = strtok (NULL, " ");
				sscanf( pch , "%d" , &face[j] );
				face[j]--;
			}
			//printf("face v %d %d %d\n" , face[0],face[1],face[2]);
			m_Faces.push_back(face);
		}
		else
		{
			continue;
		}
	}
	fclose(fin);
	m_Verts.swap(vector<CPoint3D>(m_Verts));
	m_Faces.swap(vector<CFace>(m_Faces));
	/*printf("read %d verts %d faces from file %s \n" , m_Verts.size(),m_Faces.size(), filename.c_str() );*/
	printf("read %d verts %d faces\n", m_Verts.size(), m_Faces.size());
}


void CBaseModel::ReadOffFile(const string& filename)
{
	ifstream in(filename.c_str());
	if (in.fail())
	{
		throw "fail to read file";
	}
	char buf[256];
	int vertNum, faceNum, edgeNum;
	do
	{
		in.getline(buf, sizeof buf);
		istringstream line(buf);
		if (line >> vertNum)
		{
			line >> faceNum >> edgeNum;
			break;
		}
	}while(true);

	for (int i = 0; i < vertNum; ++i)
	{
		CPoint3D pt;
		in >> pt.x;
		in >> pt.y;
		in >> pt.z;
		m_Verts.push_back(pt);
	}
	m_Verts.swap(vector<CPoint3D>(m_Verts));

	int degree;
	while (in >> degree)
	{
		int first, second;
		in >> first >> second;

		for (int i = 0; i < degree - 2; ++i)
		{
			CFace f;
			f[0] = first;
			f[1] = second;
			in >> f[2];
			m_Faces.push_back(f);			
			second = f[2];
		}
	}

	in.close();
	m_Faces.swap(vector<CFace>(m_Faces));
}

void CBaseModel::ReadMFile(const string& filename)
{
	ifstream in(filename.c_str());
	if (in.fail())
	{
		throw "fail to read file";
	}
	char buf[256];
	while (in.getline(buf, sizeof buf))
	{
		istringstream line(buf);
		if (buf[0] == '#')
			continue;
		string word;
		line >> word;
		if (word == "Vertex")
		{
			int tmp;
			line >> tmp;
			CPoint3D pt;
			line >> pt.x;
			line >> pt.y;
			line >> pt.z;

			m_Verts.push_back(pt);
		}
		else if (word == "Face")
		{
			CFace face;
			int tmp;
			line >> tmp;
			vector<int> polygon;
			polygon.reserve(4);
			while (line >> tmp)
			{
				polygon.push_back(tmp);
			}
			for (int j = 1; j < (int)polygon.size() - 1; ++j)
			{
				face[0] = polygon[0] - 1;
				face[1] = polygon[j] - 1;
				face[2] = polygon[j + 1] - 1;
				m_Faces.push_back(face);
			}
		}
		else
		{
			continue;
		}
	}
	m_Verts.swap(vector<CPoint3D>(m_Verts));
	m_Faces.swap(vector<CFace>(m_Faces));
	in.close();
}
string CBaseModel::GetFullPathAndFileName() const
{
	return m_filename;
}


void CBaseModel::Scale(double scale)
{
    for(int i = 0; i < GetNumOfVerts(); ++i){
        m_Verts[i] *= scale;
    }
}

void CBaseModel::Translate(const CPoint3D& t)
{
    for(int i = 0; i < GetNumOfVerts(); ++i){
        m_Verts[i] += t;
    }
}

void CBaseModel::FastSaveObjFile(const string& filename, const vector<pair<double, double>>& texture) const
{
	FILE* output_file = fopen(filename.c_str(), "w");
	assert(output_file != NULL);
	fprintf(output_file, "g %s\n", filename.c_str());
	for (int i = 0; i < (int)GetNumOfVerts(); ++i){
		fprintf(output_file, "v %lf %lf %lf\n", Vert(i).x, Vert(i).y, Vert(i).z);
		fprintf(output_file, "vt %lf %lf\n", texture[i].first, texture[i].second);
	}
	for (int i = 0; i < (int)GetNumOfFaces(); ++i) {
		fprintf(output_file, "f %d/%d %d/%d %d/%d\n",
			Face(i)[0] + 1, Face(i)[0] + 1,
			Face(i)[1] + 1, Face(i)[1] + 1,
			Face(i)[2] + 1, Face(i)[2] + 1);
	}
	fclose(output_file);
}


void CBaseModel::FastSaveObjFile(const string& filename) const
{

	//ofstream outFile(filename.c_str());
	FILE* output_file = fopen( filename.c_str() , "w");
    assert(output_file != NULL);
    fprintf(output_file , "g %s\n" , filename.c_str());
    for (int i = 0; i < (int)GetNumOfVerts(); ++i){
        fprintf(output_file , "v %lf %lf %lf\n" , Vert(i).x , Vert(i).y , Vert(i).z );
    }
    for (int i = 0; i < (int)GetNumOfFaces(); ++i) {
        fprintf(output_file , "f %d %d %d\n" , Face(i)[0] + 1 , Face(i)[1] + 1, Face(i)[2] + 1);
    }
    fclose(output_file);
}

void CBaseModel::FastSaveObjFileWithNormal(const string& filename) const
{

	//ofstream outFile(filename.c_str());
	FILE* output_file = fopen( filename.c_str() , "w");
    assert(output_file != NULL);
    fprintf(output_file , "g %s\n" , filename.c_str());
    for (int i = 0; i < (int)GetNumOfVerts(); ++i){
        fprintf(output_file , "v %lf %lf %lf\n" , Vert(i).x , Vert(i).y , Vert(i).z );
        fprintf(output_file , "vn %lf %lf %lf\n" , Normal(i).x , Normal(i).y , Normal(i).z);
    }
    for (int i = 0; i < (int)GetNumOfFaces(); ++i) {
        fprintf(output_file , "f %d//%d %d//%d %d//%d\n" , Face(i)[0] + 1 , Face(i)[0] + 1 ,
                                                           Face(i)[1] + 1 , Face(i)[1] + 1 ,
                                                           Face(i)[2] + 1 , Face(i)[2] + 1);
    }
    fclose(output_file);
}


void CBaseModel::FastSaveObjFileOfGivenFaces(const string& filename, const set<int>& faces) const
{
    //ofstream outFile(filename.c_str());
    FILE* output_file = fopen( filename.c_str() , "w");
    assert(output_file != NULL);
    fprintf(output_file , "g %s\n" , filename.c_str());
    for (int i = 0; i < (int)GetNumOfVerts(); ++i){
        fprintf(output_file , "v %lf %lf %lf\n" , Vert(i).x , Vert(i).y , Vert(i).z );
    }
    for (int i = 0; i < (int)GetNumOfFaces(); ++i) {
        if (faces.find(i) != faces.end()) {
            fprintf(output_file , "f %d %d %d\n" , Face(i)[0] + 1 , Face(i)[1] + 1, Face(i)[2] + 1);
        }
    }
    fclose(output_file);
}

