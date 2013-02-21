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
#ifndef _pixel_boundings_h
#define _pixel_boundings_h

#include "vec2d.h"
#include "stl_fwd.h"
#include "rectangle.h"

namespace imago
{	
	class RectShapedBounding
	{
	public:
		RectShapedBounding(const RectShapedBounding& src);
		RectShapedBounding(const Points2i& pts);

		inline const Rectangle& getBounding() const { return bound; }

	private:
		Rectangle bound;
	};
}

#endif //_pixel_boundings_h
