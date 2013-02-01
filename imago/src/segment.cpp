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

#include <deque>
#include <vector>
#include <boost/foreach.hpp>

#include "rectangle.h"
#include "segment.h"
#include "vec2d.h"
#include "segmentator.h"
#include "output.h"
#include "log_ext.h"
#include "exception.h"

using namespace imago;

Segment::Segment()
{
   _density = _ratio = -1;
   _x = _y = 0;
}

void Segment::copy( const Segment &s, bool copy_all )
{
	Image::copy(s);
	if (copy_all)
	{
		_x = s._x;
		_y = s._y;
	}
	else
	{
		_x = _y = 0;		
	}
}

/** 
* @brief Getter for x
* 
* @return const reference to _x
*/
int Segment::getX() const
{
   return _x;
}

/** 
* @brief Getter for y
* 
* @return const reference to _y
*/
int Segment::getY() const
{
   return _y;
}

/** 
* @brief Getter for x
* 
* @return const reference to _x
*/
int &Segment::getX()
{
   return _x;
}

/** 
* @brief Getter for y
* 
* @return const reference to _y
*/
int &Segment::getY()
{
   return _y;
}

Rectangle Segment::getRectangle() const
{
	return Rectangle(_x, _y, getWidth(), getHeight());
}

double Segment::getRatio() const
{
   if (_ratio < 0)
      return (double)getWidth() / getHeight();

   return _ratio;
}

double Segment::getRatio()
{
   if (_ratio < 0)
   {
      _ratio = getWidth();
      _ratio /= getHeight();
   }

   return _ratio;
}

Vec2i Segment::getCenter() const
{
	return Vec2i(_x + getWidth() / 2, _y + getHeight() / 2);
}

double Segment::getDensity() const
{
   if (_density < 0)
      return density();

   return _density;
}

double Segment::getDensity()
{
   if (_density < 0)
      _density = density();

   return _density;
}


void Segment::splitVert(int x, Segment &left, Segment &right) const
{
   Image::splitVert(x, left, right);
   
   left._x = _x;
   right._x = _x + x;
   left._y = right._y = _y;
}

void Segment::crop()
{
   int l = 0, t = 0;

   Image::crop(-1,-1,-1,-1,&l,&t);
   
   _x += l;
   _y += t;   
}

void Segment::rotate90()
{
   Image::rotate90();
   std::swap(_x, _y);
}

Segment::~Segment()
{

}
