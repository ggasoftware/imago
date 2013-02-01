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

#include <cmath>
#include <utility>

#include "boost/graph/iteration_macros.hpp"

#include "exception.h"
#include "image.h"
#include "image_draw_utils.h"
#include "segment.h"
#include "skeleton.h"
#include "vec2d.h"

using namespace imago;

#define CHECK_INDEX(i, j) ((i) >= 0 && (j) >= 0 && (i) < w && (j) < h)

void ImageDrawUtils::putLine( Image &img, double thetha, double r, double eps, byte color )
{
   int rows = img.getHeight(), cols = img.getWidth(), x, y;
   double cos_thetha = cos(thetha), sin_thetha = sin(thetha);

   for (y = 0; y < rows; y++)
      for (x = 0; x < cols; x++)
      {
         double diff = x * cos_thetha + y * sin_thetha - r;

         if (fabs(diff) < eps)
            img.getByte(x, y) = color;
      }
}

void ImageDrawUtils::putLineSegment( const Vec2i &p1, const Vec2i &p2, int color, void *userdata, bool (*plot)( int x, int y, int color, void *userdata ) )
{
   Vec2i P0 = p1, P1 = p2;

   int steep = absolute(P1.y - P0.y) > absolute(P1.x - P0.x);

   if (steep)
   {
      std::swap(P0.x, P0.y);
      std::swap(P1.x, P1.y);
   }

   int deltax = absolute(P1.x - P0.x),
      deltay = absolute(P1.y - P0.y),
      error = deltax / 2,
      ystep = P0.y < P1.y ? 1 : -1,
      xstep = P0.x > P1.x ? -1 : 1,
      y = P0.y;

   for (int x = P0.x; P0.x > P1.x ? x >= P1.x : x <= P1.x; x += xstep)
   {
      if (steep)
	  {
         plot(y, x, color, userdata); 
	  }
      else 
	  {
         plot(x, y, color, userdata);
	  }

      error -= deltay;

      if (error < 0)
      {
         y += ystep;
         error += deltax;
      }
   }
}

void ImageDrawUtils::putLineSegment( Image &img, const Vec2i &p1, const Vec2i &p2, byte color )
{
   putLineSegment(p1, p2, color, &img, _imagePlot);
}

void ImageDrawUtils::putCircle( Image& img, int cx, int cy, int r, byte color )
{
   putCircle(cx, cy, r, color, &img, _imagePlot);
}

void ImageDrawUtils::putCircle( int cx, int cy, int r, byte color, void *userdata, bool (*plot)( int x, int y, int color, void *userdata ) )
{
   int err = -r;
   int x = r;
   int y = 0;

   while (x >= y)
   {
      _plot8points(cx, cy, x, y, color, userdata, plot);

      err += y;
      ++y;
      err += y;

      if (err >= 0)
      {
         --x;
         err -= x;
         err -= x;
      }
   }
}

void ImageDrawUtils::_plot8points( int cx, int cy, int x, int y, byte color, void *userdata, bool (*plot)( int x, int y, int color, void *userdata ) )
{
   _plot4points(cx, cy, x, y, color, userdata, plot);
   if (x != y)
      _plot4points(cx, cy, y, x, color, userdata, plot);
}

void ImageDrawUtils::_plot4points( int cx, int cy, int x, int y, byte color, void *userdata, bool (*plot)( int x, int y, int color, void *userdata ) )
{
   plot(cx + x, cy + y, color, userdata);

   if (x != 0)
      plot(cx - x, cy + y, color, userdata);
   if (y != 0)
      plot(cx + x, cy - y, color, userdata);
   if (x != 0 && y != 0)
      plot(cx - x, cy - y, color, userdata);
}

bool ImageDrawUtils::_imagePlot( int x, int y, int color, void *userdata )
{
   int w, h;
   Image *img = (Image *)userdata;

   if (img == NULL)
      throw ImagoException("Null-pointer passed");

   w = img->getWidth();
   h = img->getHeight();

   if (CHECK_INDEX(x, y))
   {
      img->getByte(x, y) = color;
      return true;
   }

   return false;
}

#undef CHECK_INDEX
