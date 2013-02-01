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

#include <algorithm>
#include <cmath>

#include "rectangle.h"
#include "vec2d.h"

using namespace imago;

Rectangle::Rectangle()
{
   x = y = width = height = 0;
}

Rectangle::Rectangle( int _x, int _y, int _width, int _height )
{
   x = _x;
   y = _y;
   width = _width;
   height = _height;
}

Rectangle::Rectangle( const Vec2i &top_left, const Vec2i &bottom_right )
{
   x = top_left.x;
   y = top_left.y;
   width = bottom_right.x - x;
   height = bottom_right.y - y;
}

Rectangle::Rectangle( const Vec2i &pos, int _width, int _height )
{
   x = pos.x;
   y = pos.y;
   width = _width;
   height = _height;
}

double Rectangle::diagLength2() const
{
   return (x + width) * (x + width) + (y + height) * (y + height); 
}

double Rectangle::diagLength() const
{
   return sqrt(diagLength2());
}

double Rectangle::distance( const Rectangle &r1, const Rectangle &r2 )
{
   const Rectangle *top = &r1, *bottom = &r2;
   const Rectangle *left = top, *right = bottom;

   if (bottom->y < top->y)
      std::swap(top, bottom);
   if (right->x < left->x)
      std::swap(left, right);

   bool h_overlap, v_overlap;

   v_overlap = (top->y + top->height > bottom->y);
   h_overlap = (left->x + left->width > right->x);

   if (h_overlap && v_overlap)
      return -1;
   else if (h_overlap && !v_overlap)
      return bottom->y - top->y - top->height;
   else if (!h_overlap && v_overlap)
      return right->x - left->x - left->width;
   else
   {
      if (right == bottom)
         return Vec2d::distance(Vec2d(top->x + top->width, top->y + top->height),
                                Vec2d(bottom->x, bottom->y));
      else //left == bottom
         return Vec2d::distance(Vec2d(top->x, top->y + top->height),
                                Vec2d(bottom->x + bottom->width, bottom->y));
   }
}
