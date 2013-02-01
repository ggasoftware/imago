/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
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
#ifndef _prefilter_basic_h
#define _prefilter_basic_h

#include "image.h"
#include "settings.h"

namespace imago
{
	namespace prefilter_basic
	{	
		// returns true if result image is binarized
		// may change some pixels inensity if image is already binarized
		bool prefilterBinarizedFullsize(Settings& vars, Image &image);
  
		// filters image using cv adaptive filtering and cross-correlation
		bool prefilterBasicFullsize(Settings& vars, Image& raw);
		bool prefilterBasicForceDownscale(Settings& vars, Image& raw);		
	}
}

#endif // _prefilter_basic_h
