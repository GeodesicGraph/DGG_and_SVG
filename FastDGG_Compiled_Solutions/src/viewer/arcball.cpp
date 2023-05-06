#include "stdafx.h"
#include "arcball.h"

CArcball::CArcball( int win_width, int win_height, int ox, int oy )
{
	m_radius = (win_width < win_height )? win_width/2: win_height/2;

  m_center = Vector2D( win_width/2, win_height/2 );

  Vector2D p(ox,oy);

  _plane2sphere( p, m_position );
}

void CArcball::_plane2sphere( const Vector2D & p, Vector3D & q )
{

  Vector2D f = p;
  f /= m_radius;

  double l = sqrt( f*f );

  if( l > 1.0 ){
      q=Vector3D( f.x/l, f.y/l,0);
      return;
  }

  double fz = sqrt( 1 - l * l );

  q = Vector3D( f.x,f.y,fz );
}

CQrot CArcball::update( int nx, int ny )
{
    Vector3D position;
    _plane2sphere( Vector2D(nx,ny), position );
    Vector3D cp = m_position^position;
    CQrot r(m_position * position, cp.x,cp.y,cp.z);
    m_position = position;

    return r;
}