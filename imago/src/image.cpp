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

/**
 * @file   image.cpp
 * 
 * @brief  Implementation of Image class
 */

#include <cmath>
#include <cstring>

#include "image.h"
#include "exception.h"
#include "segment.h"

using namespace imago;

/** 
 * @brief Crops image
 */
void Image::crop(int left, int top, int right, int bottom, int* shift_x, int* shift_y)
{
   int w = getWidth();
   int h = getHeight();
   
   if (left == -1 || right == -1 || top == -1 || bottom == -1)
   {
	   for (left = 0; left < w; left++)
		   for (int y = 0; y < h; y++)
			   if (isFilled(left, y))
			   {
				   goto left_done;
			   }

	   left_done:
	   
	   for (right = w-1; right >= left; right--)
		   for (int y = 0; y < h; y++)
			   if (isFilled(right, y))
			   {
				   goto right_done;
			   }

	   right_done:
	   
	   for (top = 0; top < h; top++)
		   for (int x = 0; x < w; x++)
			   if (isFilled(x, top))
			   {
				   goto top_done;
			   }

	   top_done:
	   
	   for (bottom = h-1; bottom >= top; bottom--)
		   for (int x = 0; x < w; x++)
			   if (isFilled(x, bottom))
			   {
				   goto bottom_done;
			   }

	   bottom_done: ;
   }

   if (left >= 0 && right >= 0 && top >= 0 && bottom >= 0)
   {
	   if (shift_x) *shift_x = left;
	   if (shift_y) *shift_y = top;
	   extractRect(left, top, right, bottom, *this);
   }
}

