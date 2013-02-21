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
#ifndef _weak_segmentator_h
#define _weak_segmentator_h

#include <map>
#include <vector>
#include "image.h"
#include "basic_2d_storage.h"
#include "rectangle.h"
#include "stl_fwd.h"
#include "settings.h"

namespace imago
{
	class WeakSegmentator : public Basic2dStorage<int /*id*/>
	{
	public:		
		static Points2i getLookupPattern(int range = 1, bool fill = true);

		WeakSegmentator(int width, int height) : Basic2dStorage<int>(width, height) {}		

		// addend data from image (img.isFilled() called)
		int appendData(const Image &img, const Points2i& lookup_pattern = getLookupPattern(), bool connectMode = false);
		
		// updates crop if required
		bool needCrop(const Settings& vars, Rectangle& crop, int winSize);		

		// decorner image by setting corner pixels to 'set_to' value
		static void decorner(Image &img, byte set_to);

		typedef std::map<int, Points2i> SegMap;
		SegMap SegmentPoints;		

	protected:				
		// returns area of bounding box of segment with id
		int getRectangularArea(int id);

		// check segment with id has rectangular structure
		bool hasRectangularStructure(const Settings& vars, int id, Rectangle& bound, int winSize);
		
	private:
		// returns filled pixels count
		void fill(const Image& img, int& id, int startx, int starty, const Points2i& lookup_pattern, bool connectMode);

		// returns 2 probably condensation point for integer vector
		static bool get2centers(const std::vector<int>& data, double &c1, double& c2);		
	};
}

#endif //_weak_segmentator_h
