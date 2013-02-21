/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
 *
 * This file is part of Imago OCR project.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#include <limits.h>

#include "pixel_boundings.h"
#include "settings.h"

namespace imago
{
	RectShapedBounding::RectShapedBounding(const RectShapedBounding& src)
	{
		bound = src.bound;
	}

	RectShapedBounding::RectShapedBounding(const Points2i& pts)
	{
		int min_x = INT_MAX, min_y = INT_MAX, max_x = 0, max_y = 0;
		for (Points2i::const_iterator it = pts.begin(); it != pts.end(); ++it)
		{
			min_x = std::min(min_x, it->x);
			min_y = std::min(min_y, it->y);
			max_x = std::max(max_x, it->x);
			max_y = std::max(max_y, it->y);
		}
		bound = Rectangle(min_x, min_y, max_x, max_y, 0);
	}
}
