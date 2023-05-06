
#include "stdafx.h"
#include "Quat.h"

void CQrot::normalize()
{
  double l = m_w * m_w + m_x * m_x + m_y * m_y + m_z * m_z;
    if( l < 1e-20 ) {  
        m_x = 0; m_y = 0; m_z = 0; m_w = 1;
        return;
    }

    l = sqrt(l);

    m_w /= l;
    m_x /= l;
    m_y /= l;
    m_z /= l;
}

CQrot & CQrot::operator^(double p)
{
  normalize();
  double theta = 2 * acos( m_w );
  if( theta < 1e-10 ) return (*this); 

  Vector3D axis( m_x,m_y,m_z );
  axis.normalize();
  theta *= p;
  m_w   = cos( theta * 0.5 );
  axis *= sin( theta * 0.5 );

  m_x = axis.x;
  m_y = axis.y;
  m_z = axis.z;

  return (*this);
}

Vector3D  CQrot::operator*(  const Vector3D & p )
{
    CQrot   q(m_w,m_x,m_y,m_z);
    CQrot pq( 0, p.x,p.y, p.z);
    CQrot iq = q^(-1);
    CQrot r =     q *  pq * iq ;
    return Vector3D(r.m_x,r.m_y, r.m_z);
}


CQrot operator^(const CQrot & r, double p)
{
  CQrot q = r;
  q.normalize();
  double theta = 2 * acos( q.m_w );
  if( theta < 1e-10 ) return q; 

  Vector3D axis( q.m_x, q.m_y, q.m_z );
  axis.normalize();
  theta *= p;
  q.m_w   = cos( theta * 0.5 );
  axis *= sin( theta * 0.5 );

  q.m_x = axis.x;
  q.m_y = axis.y;
  q.m_z = axis.z;

  return q;
}


CQrot operator*( const CQrot & p, const CQrot & q )
{

    double   sp = p.m_w;
    Vector3D vp( p.m_x, p.m_y, p.m_z );

    double   sq = q.m_w;
    Vector3D vq( q.m_x, q.m_y, q.m_z );
 
    double   sr = sp * sq - vp * vq;

    Vector3D vr;
    vr = sq * vp + sp * vq  + (vp^vq);

    return CQrot( sr, vr.x, vr.y, vr.z);
}


CQrot operator+( const CQrot & p, const CQrot & q )
{
  return CQrot( p.m_w + q.m_w, p.m_x + q.m_x, p.m_y + q.m_y, p.m_z + q.m_z);
}

CQrot operator-( const CQrot & p, const CQrot & q )
{
  return CQrot( p.m_w - q.m_w, p.m_x - q.m_x, p.m_y - q.m_y, p.m_z - q.m_z);
}

CQrot operator*( const CQrot & p, double s )
{
  return CQrot( p.m_w * s, p.m_x * s, p.m_y * s, p.m_z *s);
}

double operator^( const CQrot & p, const CQrot & q )
{
  return ( p.m_w * q.m_w + p.m_x * q.m_x + p.m_y * q.m_y + p.m_z * q.m_z);
}

void CQrot::convert( double *  m)
{
  CQrot q = *this;
  double l = q^q;
  double s = 2.0 / l;
  double xs = q.m_x*s;
  double ys = q.m_y*s;
  double zs = q.m_z*s;
  
  double wx = q.m_w*xs;
  double wy = q.m_w*ys;
  double wz = q.m_w*zs;
  
  double xx = q.m_x*xs;
  double xy = q.m_x*ys;
  double xz = q.m_x*zs;
  
  double yy = q.m_y*ys;
  double yz = q.m_y*zs;
  double zz = q.m_z*zs;
  

  m[0*4+0] = 1.0 - (yy + zz);
  m[1*4+0] = xy - wz;
  m[2*4+0] = xz + wy;
  m[0*4+1] = xy + wz;
  m[1*4+1] = 1.0 - (xx + zz);
  m[2*4+1] = yz - wx;
  m[0*4+2] = xz - wy;
  m[1*4+2] = yz + wx;
  m[2*4+2] = 1.0 - (xx + yy);

  
  m[0*4+3] = 0.0;
  m[1*4+3] = 0.0;
  m[2*4+3] = 0.0;
  m[3*4+0] = m[3*4+1] = m[3*4+2] = 0.0;
  m[3*4+3] = 1.0;
}

