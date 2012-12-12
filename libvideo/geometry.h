// $Id: geometry.h,v 1.1.1.1.2.2 2005/02/06 00:44:02 cvs Exp $
//

#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

class Geometry
{
public:
  static double normalizeAngle (double angle);
  static double normalizeDirection (double angle);
  static double angleDifference (double angle1, double angle2);
  static double angleSum (double angle1, double angle2);
  static double intermediateAngle (double angle1, double angle2);
  static double distToLine (double x, double y, double a, double b);
  static void intersectLines (int n, double dirt[], double offt[],
			      double dirl[], double offl[], double *x,
			      double *y);
  static int clamp (int min, int val, int max);
  static double clamp ( double min, double val, double max);
  static double dist2D_Segment_to_Segment( double x11, double y11, double x12, double y12, double x21, double y21, double x22, double y22 );
  static void minMax( double x1, double x2, double * min, double * max );
  static void minMax( unsigned int x1, unsigned int x2, unsigned int * min, unsigned int * max );
  static unsigned int manhattenDistance( int x1, int y1, int x2, int y2 );
  static unsigned int circlePixelArea(double radius, double pixelDim);
  static unsigned int squarePixelArea(double dimension, double pixelDim);
  static unsigned int rectPixelArea(double x, double y, double pixelDim);
};
#endif
