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

#pragma once
#ifndef _segment_tools_h
#define _segment_tools_h

#include "stl_fwd.h"
#include "segment.h"

namespace imago
{
   namespace SegmentTools
   {
		// return all filled points from segment
		Points2i getAllFilled(const Segment& seg);
		
		// return count of filled points
		int getFilledCount(const Segment& seg);

		// returns distance between two sets
		enum DistanceType { dtEuclidian, dtDeltaX, dtDeltaY };
		double getRealDistance(const Segment& seg1, const Segment& seg2, DistanceType type = dtEuclidian);

		// returns real segment height (delta between top and bottom filled pixels)
		int getRealHeight(const Segment& seg);

		// returns percentage of pixels with y > line_y
		double getPercentageUnderLine(const Segment& seg, int line_y);

		// returns all filled pixels in range of [range x range] from pos
		Points2i getInRange(const Image& seg, Vec2i pos, int range);
		
		// returns all endpoints
		Points2i getEndpoints(const Segment& seg);
		
		// return nearest pixel of pts from start point
		Vec2i getNearest(const Vec2i& start, const Points2i& pts);
   };
}

#endif // _segment_tools_h
