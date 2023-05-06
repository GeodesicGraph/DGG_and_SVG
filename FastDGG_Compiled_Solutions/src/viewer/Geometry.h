// Geometry.h: classes of 2D vector and 3D vector
//
//////////////////////////////////////////////////////////////////////

#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

#include <math.h>
#define DOUBLE_EPS 1e-16
// double zero eps
#define	EQUALZERO(x)	(fabs((x)) < DOUBLE_EPS)
using namespace std;




/////////////////////////////////////////////////////////////
// Vector2D : 2D vector
/////////////////////////////////////////////////////////////
class Vector2D  
{
public :
	double x,y;

public:
	Vector2D(){	x = 0;	y = 0;}
	// constructions
	Vector2D(double xx,double yy)	{	x = xx;	y = yy;}
	Vector2D(const Vector2D& v)	{	x = v.x;	y = v.y;}

  
	// operator
	double	  length()		{	return sqrt(x*x + y*y);	}
	double	  length2()		{	return x*x + y*y;	}
	double	  normalize()	{	double len = length();	if (!EQUALZERO(len)) {x/= len;y/=len;}	return len;	}
	Vector2D& operator=(const Vector2D& v);
	Vector2D& operator+=(const Vector2D& v);
	Vector2D& operator-=(const Vector2D& v);
	Vector2D& operator*=(double u);
	Vector2D& operator/=(double u);
//	Vector2D& operator^=(const Vector2D& v);

	bool	Intersect(Vector2D v1,Vector2D v2,Vector2D v3,Vector2D v4);
	bool	Intersect(Vector2D v1,Vector2D v2);

	friend Vector2D operator+(const Vector2D& lv, const Vector2D& rv);
	friend Vector2D operator-(const Vector2D& lv, const Vector2D& rv);
	friend Vector2D operator*(const double u, const Vector2D& rv);
	friend Vector2D operator*(const Vector2D& lv, const double u);
	friend Vector2D operator/(const Vector2D& lv, const double u);
	friend double   operator*(const Vector2D& lv, const Vector2D& rv);
//	friend Vector2D operator^(const Vector2D& lv, const Vector2D& rv);

	short	AtWhere(Vector2D v0,Vector2D v1);
	bool	AtRight(Vector2D v0,Vector2D v1);
	bool	AtLeft(Vector2D v0,Vector2D v1);
	bool	OnLine(Vector2D v0,Vector2D v1);
	double	GetArea(Vector2D v);
	

};

/////////////////////////////////////////////////////////////
// Vector3D : 3D vector
/////////////////////////////////////////////////////////////
class Vector3D 
{
public :
	double x,y,z;

	// constructions
	Vector3D()	{	x = 0;	y = 0;	z = 0;	}
	Vector3D(double xx,double yy,double zz)	{	x = xx;	y = yy;	z = zz;	}
	Vector3D(const Vector3D& v)	{	x = v.x;	y = v.y;	z = v.z;	}

	// operator
	double	  length()		{	return sqrt(x*x + y*y + z*z);	}
	double	  length2()		{	return x*x + y*y + z*z;	}
	double	  normalize()	{	double len = length();	if (!EQUALZERO(len)) {x/= len;y/=len;z/=len;}	return len;	}
	double	  DisFrom( Vector3D v )  { return sqrt( (x-v.x)*(x-v.x)+(y-v.y)*(y-v.y)+(z-v.z)*(z-v.z) ); }
	Vector3D& operator=(const Vector3D& v);
	Vector3D& operator+=(const Vector3D& v);
	Vector3D& operator-=(const Vector3D& v);
	Vector3D& operator*=(double u);
	Vector3D& operator/=(double u);
	Vector3D& operator^=(const Vector3D& v);
	bool operator<(const Vector3D& v)const;

	friend Vector3D operator+(const Vector3D& lv, const Vector3D& rv);
	friend Vector3D operator-(const Vector3D& lv, const Vector3D& rv);
	friend Vector3D operator*(const double u, const Vector3D& rv);
	friend Vector3D operator*(const Vector3D& lv, const double u);
	friend Vector3D operator/(const Vector3D& lv, const double u);
	friend double   operator*(const Vector3D& lv, const Vector3D& rv);
	friend Vector3D operator^(const Vector3D& lv, const Vector3D& rv);
	void print(const string& info = "")const{
		if( info != "" ){
			printf("%s : " , info.c_str());
		}
		printf("%lf %lf %lf\n" ,x,y,z);
	}

};


#endif 
