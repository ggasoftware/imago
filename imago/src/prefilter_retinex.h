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
#ifndef _prefilter_retinex_h
#define _prefilter_retinex_h

#include "image.h"
#include "settings.h"

namespace imago
{
	namespace prefilter_retinex
	{
		// filters image using retinex-based approach
		bool prefilterRetinexDownscaleOnly(Settings& vars, Image& raw);
		bool prefilterRetinexFullsize(Settings& vars, Image& raw);
	}
}

#endif // _prefilter_retinex_h
