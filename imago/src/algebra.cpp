/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
 * 
 * This file is part of Imago toolkit.
 * 
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#include <math.h>

#include "algebra.h"
#include "rectangle.h"
#include "exception.h"

using namespace imago;

double Algebra::distance2segment( const Vec2d &p, const Vec2d &b, const Vec2d &e )
{
   double c1, c2, c3;
   Vec2d u, v, w, h;

   v.diff(e, b);
   u.diff(p, b);

   //angle is obtuse
   if ((c1 = Vec2d::dot(v, u)) < 0)
      return u.norm();

   //projection is longer than segment
   if ((c2 = Vec2d::dot(v, v)) < c1)
   {
      w.diff(p, e);
      return w.norm();
   }

   //general case
   c3 = c1 / c2;

   h.diff(p, b);
   v.scale(c3);
   h.sub(v);

   return h.norm();
}

double Algebra::distance2rect( const Vec2d &p, int x, int y, int width, int height )
{
   Vec2d tl(x, y), tr(x + width, y),
         dl(x, y + height), dr(x + width, y + height);

   if ((p.x >= x && p.x <= x + width) &&
       (p.y >= y && p.y <= y + height))
      return 0;

   double d[4];
   d[0] = Algebra::distance2segment(p, tl, tr);
   d[1] = Algebra::distance2segment(p, tr, dr);
   d[2] = Algebra::distance2segment(p, dl, dr);
   d[3] = Algebra::distance2segment(p, dl, tl);

   d[0] = std::min(d[0], d[1]);
   d[0] = std::min(d[0], d[2]);
   d[0] = std::min(d[0], d[3]);

   return d[0];
}

double Algebra::distance2rect( const Vec2d &p, const Rectangle &r )
{
   return distance2rect(p, r.x, r.y, r.width, r.height);
}

Line Algebra::points2line( const Vec2d &b, const Vec2d &e )
{
   Line l;
   l.A = b.y - e.y;
   l.B = e.x - b.x;
   l.C = b.x * e.y - e.x * b.y;
   double s = sqrt(l.A * l.A + l.B * l.B);
   l.A /= s;
   l.B /= s;
   l.C /= s;
   return l;
}

Vec2d Algebra::linesIntersection(const Settings& vars, const Vec2d &p11, const Vec2d &p12,
                                  const Vec2d &p21, const Vec2d &p22 )
{
   Line l1 = points2line(p11, p12);
   Line l2 = points2line(p21, p22);
   double den = l1.A * l2.B - l2.A * l1.B;

   if (absolute(den) < vars.routines.Algebra_IntersectionEps)
   {
      Vec2d res;
      res.add(p11);
      res.add(p21);
      res.scale(0.5); // average
      return res;
   }
   else
   {
      return linesIntersection(vars, l1, l2);
   }
}

Vec2d Algebra::linesIntersection(const Settings& vars, const Line &l1, const Line &l2 )
{
   double den = l1.A * l2.B - l2.A * l1.B; 

   if (absolute(den) < vars.routines.Algebra_IntersectionEps) 
      throw DivizionByZeroException("linesIntersection");

   Vec2d res;
   res.x = (l1.B * l2.C - l2.B * l1.C) / den;
   res.y = (l1.C * l2.A - l2.C * l1.A) / den;

   return res;
}

double Algebra::slope( const Vec2d &b, const Vec2d &e )
{
   Vec2d dxdy;
   dxdy.diff(e, b);
   if (absolute(dxdy.x) < EPS)
      return HALF_PI;
   else
      return atan((double) dxdy.y / dxdy.x);
}

double Algebra::pointProjectionCoef(const Vec2d &orig,
                                    const Vec2d &to_begin, const Vec2d &to_end )
{
   Vec2d a, b;
   a.diff(to_end, to_begin);
   b.diff(orig, to_begin);

   double ab = Vec2d::dot(a, b);
   double aa = Vec2d::dot(a, a);

   if (aa < EPS)
      throw DivizionByZeroException("pointProjectionCoef");

   return ab / aa;
}


std::pair<Vec2d, Vec2d> Algebra::segmentProjection(
                        const Vec2d &orig_begin, const Vec2d &orig_end,
                        const Vec2d &to_begin, const Vec2d &to_end )
{
   Vec2d v1, v2, v3;
   double a1, a2;
   double l1 = Vec2d::distance(to_begin, to_end);

   v1.diff(to_end, to_begin);
   v2.diff(orig_end, to_begin);
   v3.diff(orig_begin, to_begin);

   a1 = Vec2d::dot(v1, v2) / l1;
   a2 = Vec2d::dot(v1, v3) / l1;
   v1 = v1.getNormalized();
   v2.scaled(v1, a1);
   v3.scaled(v1, a2);

   v2.add(to_begin);
   v3.add(to_begin);

   return std::make_pair(v3, v2);
}

bool Algebra::segmentsParallel( const Vec2d &b1, const Vec2d &e1,
                                const Vec2d &b2, const Vec2d &e2,
                                double eps, double *dist )
{
   Line l1 = points2line(b1, e1);
   Line l2 = points2line(b2, e2);

   double den = l1.A * l2.B - l2.A * l1.B;

   if (absolute(den) > eps)
      return false;

   if (dist)
   {
      double k = 0;
      if (absolute(l2.A) > 0.01)
         k = absolute(l1.A / l2.A);
      else if (absolute(l2.B) > 0.01)
         k = absolute(l1.B / l2.B);
      else
         k = 1.0; //this is strange
      
      double d = (l1.C - k * l2.C);
      *dist = absolute(d);
   }
   return true;
}

bool Algebra::rangesSeparable (double a1, double a2, double b1, double b2)
{
   double a_min = std::min(a1, a2);
   double a_max = std::max(a1, a2);

   double b_min = std::min(b1, b2);
   double b_max = std::max(b1, b2);

   if (a_min < b_min)
      return a_max < b_min;

   return a_min > b_max;
}

bool Algebra::SegmentsOnSameLine(const Settings& vars, Vec2d &b1, Vec2d &e1, 
								Vec2d &b2, Vec2d &e2)
{
	Vec2d bdif, edif;
	bdif.diff(b1, b2);
	edif.diff(e1, e2);

	if(Algebra::segmentsParallel(b1, e1, b2, e2, vars.routines.Algebra_SameLineEps) &&
		Algebra::segmentsParallel(bdif, edif, b2, e2, vars.routines.Algebra_SameLineEps))
		return true;
	return false;
}