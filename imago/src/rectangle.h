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

#pragma once
#ifndef _rectangle_h
#define _rectangle_h

#include "vec2d.h"

namespace imago
{
   class Rectangle
   {
   public:

      Rectangle();
      Rectangle( int _x, int _y, int _width, int _height );
      Rectangle( const Vec2i &top_left, const Vec2i &bottom_right );
      Rectangle( const Vec2i &pos, int _width, int _height );
      
      double diagLength2() const;
      double diagLength() const;
      static double distance( const Rectangle &r1, const Rectangle &r2 );

      int x, y;
      int width, height;

	  int x1() const { return x; }
	  int x2() const { return x + width; }
	  int y1() const { return y; }
	  int y2() const { return y + height; }

	  void adjustBorder(int border)
	  {
		  x += border;
		  y += border;
		  width -= 2*border;
		  height -= 2*border;
	  }

	  Rectangle( int x1, int y1, int x2, int y2, int border )
	  {
		  x = x1;
		  y = y1;
		  width  = x2 - x1;
		  height = y2 - y1;
		  adjustBorder(border);
	  }
   };
}


#endif /* _rectangle_h */

