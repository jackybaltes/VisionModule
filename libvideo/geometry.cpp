// $Id: geometry.cpp,v 1.1.1.1.2.2 2005/02/06 00:44:02 cvs Exp $
//

using namespace std;

#include <iostream>
#include <math.h>
#include "geometry.h"
#include <stdlib.h>

double
Geometry::normalizeAngle (double angle)
{
  while (angle >= 2 * M_PI)
    {
      angle = angle - 2 * M_PI;
    }
  while (angle < -2 * M_PI)
    {
      angle = angle + 2 * M_PI;
    }
  if (angle >= M_PI)
    {
      angle = angle - 2 * M_PI;
    }
  else if (angle < -M_PI)
    {
      angle = 2 * M_PI + angle;
    }
  return angle;
}

double
Geometry::normalizeDirection (double angle)
{
  double a = normalizeAngle (angle);

  if (a < 0)
    {
      a = a + M_PI;
    }
  return a;
}

double
Geometry::angleDifference (double angle1, double angle2)
{
  double a1, a2;
  double diff;

  a1 = normalizeAngle (angle1);
  a2 = normalizeAngle (angle2);

  diff = a1 - a2;
  diff = normalizeAngle (diff);
  return diff;
}

double
Geometry::angleSum (double angle1, double angle2)
{
  double sum;

  sum = angle1 + angle2;
  sum = normalizeAngle (sum);
  return sum;
}

double
Geometry::intermediateAngle (double angle1, double angle2)
{
  double diff;

  diff = angleDifference (angle1, angle2);
  if (diff > M_PI / 2)
    {
      diff = diff - M_PI;
    }
  else if (diff <= -M_PI / 2)
    {
      diff = -M_PI - diff;
    }
  return diff;
}

void
Geometry::intersectLines (int /* n */,
			  double * /* dir1 */, double * /* off1[] */,
			  double * /* dir2[] */, double * /* off2[] */, double * /* x */, double * /* y */)
{
  cerr << "intersectLines not implemented yet\n";

#if 0
  double mrow0[] = { 0.0, 0.0 };
  double mrow1[] = { 0.0, 0.0 };
  double *m[] = { mrow0, mrow1 };

  double brow0[] = { 0.0 };
  double brow1[] = { 0.0 };
  double *b[] = { brow0, brow1 };
  char *msg;
  int i;
  double diff_x, diff_y;

  for (i = 0; i < n; i++)
    {
      m[i][0] = dir1[i];
      m[i][1] = -dir2[i];
      b[i][0] = off2[i] - off1[i];
    }
  if ((msg = gaussj (m, 2, b, 1)) != NULL)
    {
      fprintf (stderr, "intersect_lines: gaussj returns %s\n", msg);
    }
#ifdef DEBUG
  fprintf (stdout, "gaussj: returns %f, %f\n", b[0][0], b[1][0]);
#endif
  diff_x = 0.0;
  diff_y = 0.0;
  if (((diff_y =
	fabs (dir1[0] * b[0][0] + off1[0] - dir2[0] * b[1][0] - off2[0])) >
       1.0)
      ||
      ((diff_x =
	fabs (dir1[1] * b[0][0] + off1[1] - dir2[1] * b[1][0] - off2[1])) >
       1.0))
    {
      fprintf (stderr,
	       "intersect_lines: the two lines do not seem to intersect (%f,%f)\n",
	       diff_y, diff_x);
      fprintf (stdout,
	       "intersect_lines: the two lines do not seem to intersect (%f,%f)\n",
	       diff_y, diff_x);
    }

  *y = dir1[0] * b[0][0] + off1[0];
  *x = dir1[1] * b[0][0] + off1[1];

#ifdef DEBUG
  fprintf (stdout, "intersect_lines: returns %f, %f\n", *x, *y);
#endif
#endif
}


double
Geometry::distToLine (double x, double y, double a, double b)
{
  double dist;

  dist = (a * x + b - y) / sqrt (1 + a * a);
  return dist;
}

int
Geometry::clamp (int min, int val, int max)
{
  int ret = val;
  if (val < min)
    {
      ret = min;
    }
  else if (val > max)
    {
      ret = max;
    }
  return ret;
}

double
Geometry::clamp ( double min, double val, double max )
{
  double ret = val;
  if (val < min)
    {
      ret = min;
    }
  else if (val > max)
    {
      ret = max;
    }
  return ret;
}

// dist3D_Segment_to_Segment():
//    Input:  two 3D line segments S1 and S2
//    Return: the shortest distance between S1 and S2
double
Geometry::dist2D_Segment_to_Segment( double x11, double y11, double x12, double y12, double x21, double y21, double x22, double y22 )
{
  double u_x = x12 - x11;
  double u_y = y12 - y11;

  double v_x = x22 - x21;
  double v_y = y22 - y21;

  double w_x = x11 - x21;
  double w_y = y11 - y21;

  double a = u_x * u_x + u_y * u_y;
  double b = u_x * v_x + u_y * v_y;
  double c = v_x * v_x + v_y * v_y;
  double d = u_x * w_x + u_y * w_y;
  double e = v_x * w_x + v_y * w_y;

  double D = a * c - b * b;

  //    float    sc, sN, sD = D;      // sc = sN / sD, default sD = D >= 0
  //    float    tc, tN, tD = D;      // tc = tN / tD, default tD = D >= 0

  double sc, sN, sD = D;
  double tc, tN, tD = D;

    // compute the line parameters of the two closest points
  if (D < 0.1 ) 
    { // the lines are almost parallel
      sN = 0.0;        // force using point P0 on segment S1
      sD = 1.0;        // to prevent possible division by 0.0 later
      tN = e;
      tD = c;
    }
  else 
    {                // get the closest points on the infinite lines
      sN = (b*e - c*d);
      tN = (a*e - b*d);
      if (sN < 0.0) {       // sc < 0 => the s=0 edge is visible
	sN = 0.0;
	tN = e;
	tD = c;
      }
      else if (sN > sD) 
	{  // sc > 1 => the s=1 edge is visible
	  sN = sD;
	  tN = e + b;
	  tD = c;
        }
    }

  if (tN < 0.0) 
    {           // tc < 0 => the t=0 edge is visible
      tN = 0.0;
      // recompute sc for this edge
      if (-d < 0.0)
	{
	  sN = 0.0;
	}
      else if (-d > a)
	{
	  sN = sD;
	}
      else 
	{
	  sN = -d;
	  sD = a;
	}
    }
  else if (tN > tD) 
    {      // tc > 1 => the t=1 edge is visible
      tN = tD;
      // recompute sc for this edge
      if ((-d + b) < 0.0)
	{
	  sN = 0;
	}
      else if ((-d + b) > a)
	{
	  sN = sD;
	}
      else 
	{
	  sN = (-d + b);
	  sD = a;
        }
    }
  // finally do the division to get sc and tc
  sc = ( fabs(sN) < 0.1 ? 0.0 : sN / sD);
  tc = ( fabs(tN) < 0.1 ? 0.0 : tN / tD);
  
  // get the difference of the two closest points
  double dP_x = w_x + sc * u_x - tc * v_x;
  double dP_y = w_y + sc * u_y - tc * v_y;
  
  return hypot( dP_x, dP_y );
}

void Geometry::minMax( unsigned int x1, unsigned int x2, unsigned int * min, unsigned int * max )
{
  if ( x1 >= x2 )
    {
      if ( min != 0 )
	{
	  * min = x2;
	}
      if ( max != 0 )
	{
	  * max = x1; 
	}
    }
  else
    {
      if ( min != 0 )
	{
	  * min = x1;
	}
      if ( max != 0 )
	{
	  * max = x2;
	}
    }
}

void
Geometry::minMax( double x1, double x2, double * min, double * max )
{
  if ( x1 >= x2 )
    {
      if ( min != 0 )
	{
	  * min = x2;
	}
      if ( max != 0 )
	{
	  * max = x1; 
	}
    }
  else
    {
      if ( min != 0 )
	{
	  * min = x1;
	}
      if ( max != 0 )
	{
	  * max = x2;
	}
    }
}

unsigned int
Geometry::manhattenDistance( int x1, int y1, int x2, int y2 )
{
  return abs( x1 - x2 ) + abs( y1 - y2 );
}

// these are not very sophisticated approximations
unsigned int Geometry::circlePixelArea(double radius, double pixelDim)
{
  unsigned int area = 0;
  if(pixelDim > 0)
    {
      area = (unsigned int)(ceil((M_PI * radius * radius)/(pixelDim*pixelDim)));
    }
  return area;
}
unsigned int Geometry::squarePixelArea(double dimension, double pixelDim)
{
  return rectPixelArea(dimension,dimension, pixelDim);
}
unsigned int Geometry::rectPixelArea(double x, double y, double pixelDim)
{
  unsigned int area = 0;
  if(pixelDim > 0)
    {
      area = (unsigned int)ceil((x * y)/(pixelDim * pixelDim));
    }

  return area;
}
