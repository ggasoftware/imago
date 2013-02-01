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
#ifndef _prefilter_entry_h
#define _prefilter_entry_h

#include "image.h"
#include "settings.h"

namespace imago
{
	// selects first OK prefilter
	bool prefilterEntrypoint(Settings& vars, Image& output, const Image& src);
	
	// iterates trough next filters
	bool applyNextPrefilter(Settings& vars, Image& output, const Image& src, bool iterateNext = true);

	namespace PrefilterUtils
	{
		// returns true if image was modified
		bool resampleImage(const Settings& vars, Image &image);
	}
}

#endif //_prefilter_entry_h


