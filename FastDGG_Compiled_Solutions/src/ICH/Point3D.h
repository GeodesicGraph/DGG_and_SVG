// CPoint3D.h: interface for the CPoint3D class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_POINT3D_H__2655648F_9CC2_42AA_9582_9108241B1220__INCLUDED_)
#define AFX_POINT3D_H__2655648F_9CC2_42AA_9582_9108241B1220__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdafx.h"
using namespace std;
struct CPoint3D  
{
public:
	static const CPoint3D MAX_VALUE;
	double x, y, z;
	CPoint3D();
	CPoint3D(double* xyz);
	CPoint3D(double x, double y, double z);	
	inline CPoint3D& operator +=(const CPoint3D& pt);
	inline CPoint3D& operator -=(const CPoint3D& pt);
	inline CPoint3D& operator *=(double times);
	inline CPoint3D& operator /=(double times);
	inline CPoint3D operator /(double times) const;
	inline bool operator==(const CPoint3D& pt)const;
	inline bool operator!=(const CPoint3D& pt)const;
	inline double Len() const;
	inline double LenSqr() const;
	inline CPoint3D& CPoint3D::Normalize();
	inline void Print(const string& s = "" ) const;
	inline void Show() const;
	inline void ShowWithNormal(const CPoint3D& normal)const;
	inline void SetColor() const;
	inline bool equal(const CPoint3D& p) const;
};

bool CPoint3D::equal(const CPoint3D& p) const 
{
	if ( fabs(x-p.x) < 1e-8 && fabs(y-p.y) < 1e-8 && fabs(z-p.z) < 1e-8) {
		return true;
	}
	return false;
}


CPoint3D& CPoint3D::operator +=(const CPoint3D& pt)
{
	x += pt.x;
	y += pt.y;
	z += pt.z;
	return *this;
}

CPoint3D& CPoint3D::operator -=(const CPoint3D& pt)
{
	x -= pt.x;
	y -= pt.y;
	z -= pt.z;
	return *this;
}

CPoint3D& CPoint3D::operator *=(double times)
{
	x *= times;
	y *= times;
	z *= times;
	return *this;
}

CPoint3D& CPoint3D::operator /=(double times)
{
	x /= times;
	y /= times;
	z /= times;
	return *this;
}

bool CPoint3D::operator==(const CPoint3D& pt) const
{
	return fabs(x - pt.x) < DOUBLE_EPSILON && fabs(y - pt.y) < DOUBLE_EPSILON && fabs(z - pt.z) < DOUBLE_EPSILON;
}

bool CPoint3D::operator!=(const CPoint3D& pt) const
{
	return fabs(x - pt.x) > DOUBLE_EPSILON || fabs(y - pt.y) > DOUBLE_EPSILON || fabs(z- pt.z) > DOUBLE_EPSILON;
	//return x != pt.x || y != pt.y || z != pt.z;
}

CPoint3D CPoint3D::operator /(double times) const
{
	return CPoint3D(x / times, y / times, z / times);
}

double CPoint3D::Len() const
{
	return sqrt(x * x + y * y + z * z);
}
double CPoint3D::LenSqr() const
{
	return x * x + y * y + z * z;
}

CPoint3D& CPoint3D::Normalize()
{
	double len = Len();
	x /= len;
	y /= len;
	z /= len;
	return *this;
}

void CPoint3D::Print(const string& s) const
{
	printf( "%s: x %10e y %10e z %10e\n" , s.c_str() , x , y , z );
}

void CPoint3D::Show()const
{
	glVertex3f((GLfloat)x , (GLfloat)y , (GLfloat)z);
}

void CPoint3D::ShowWithNormal(const CPoint3D& normal)const
{
	glNormal3f( (GLfloat)normal.x , (GLfloat)normal.y , (GLfloat)normal.z);
	glVertex3f((GLfloat)x , (GLfloat)y , (GLfloat)z);
}

void CPoint3D::SetColor() const
{
	glColor3f((GLfloat)x , (GLfloat)y , (GLfloat)z);
}

CPoint3D operator +(const CPoint3D& pt1, const CPoint3D& pt2);
CPoint3D operator -(const CPoint3D& pt1, const CPoint3D& pt2);
CPoint3D operator *(const CPoint3D& pt, double times);
CPoint3D operator *(double times, const CPoint3D& pt);
CPoint3D operator *(const CPoint3D& pt1, const CPoint3D& pt2);
CPoint3D VectorCross(const CPoint3D& pt1, const CPoint3D& pt2, const CPoint3D& pt3);
double operator ^(const CPoint3D& pt1, const CPoint3D& pt2);
double GetTriangleArea(const CPoint3D& pt1, const CPoint3D& pt2, const CPoint3D& pt3);
double AngleBetween(const CPoint3D& pt1, const CPoint3D& pt2);
double AngleBetween(const CPoint3D& pt1, const CPoint3D& pt2, const CPoint3D& pt3);
void VectorCross(const float* u, const float* v, float * n);
float VectorDot(const float* u, const float* v);
float AngleBetween(const float* u, const float* v);
#endif // !defined(AFX_POINT3D_H__2655648F_9CC2_42AA_9582_9108241B1220__INCLUDED_)
